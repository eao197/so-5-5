/*
 * A test for layers.
 */

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <exception>

#include <ace/OS.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <cpp_util_2/h/defs.hpp>

#include <so_5/rt/h/rt.hpp>
#include <so_5/rt/h/so_layer.hpp>

#include <so_5/api/h/api.hpp>

#include <utest_helper_1/h/helper.hpp>

namespace separate_so_thread
{

ACE_THR_FUNC_RETURN
entry_point( void * env )
{
	try
	{
		so_5::rt::so_environment_t * so_env =
			reinterpret_cast< so_5::rt::so_environment_t * >( env );

		so_env->run();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		std::abort();
	}

	return 0;
}

typedef std::map<
		so_5::rt::so_environment_t *,
		ACE_thread_t >
	env_tid_map_t;

env_tid_map_t		g_env_tid_map;
ACE_Thread_Mutex	g_lock;

void
start( so_5::rt::so_environment_t & env )
{
	ACE_Guard< ACE_Thread_Mutex > lock( g_lock );

	if( g_env_tid_map.end() == g_env_tid_map.find( &env ) )
	{
		ACE_thread_t tid;

		if( -1  == ACE_Thread_Manager::instance()->spawn(
			entry_point,
			&env,
			THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
			&tid ) )
		{
			throw std::runtime_error( "unable_to start so thread" );
		}

		g_env_tid_map[ &env ] = tid;
	}
}

void
wait( so_5::rt::so_environment_t & env )
{
	ACE_Guard< ACE_Thread_Mutex > lock( g_lock );

	env_tid_map_t::iterator it = g_env_tid_map.find( &env );
	if( g_env_tid_map.end() != it )
	{
		ACE_Thread_Manager::instance()->join( it->second );
		g_env_tid_map.erase( it );
	}
}

class init_finish_signal_mixin_t
{
	public :
		init_finish_signal_mixin_t()
			:	m_init_finish_signal( m_init_finish_lock )
		{
			m_init_finish_lock.acquire();
		}

		void
		wait_for_init_finish()
		{
			m_init_finish_signal.wait( m_init_finish_lock );
		}

	protected :
		void
		init_finished()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_init_finish_lock );
			m_init_finish_signal.signal();
		}

	private :
		ACE_Thread_Mutex m_init_finish_lock;
		ACE_Condition_Thread_Mutex m_init_finish_signal;
};

} /* namespace separate_so_thread */

std::array< so_5::rt::so_layer_t *, 64 > last_created_objects;

template < int N >
class test_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:
		test_layer_t()
		{
			last_created_objects[ N ] = &*this;
		}

		virtual ~test_layer_t()
		{}

		virtual so_5::ret_code_t
		start()
		{
			return 0;
		}

		virtual void
		shutdown()
		{}

		virtual void
		wait()
		{}
};

class so_environment_t
	:	public so_5::rt::so_environment_t
	,	public separate_so_thread::init_finish_signal_mixin_t
{
		typedef so_5::rt::so_environment_t base_type_t;
	public:
		so_environment_t(
			test_layer_t< 1 > * tl1,
			test_layer_t< 2 > * tl2,
			test_layer_t< 3 > * tl3 )
			:
				base_type_t(
					so_5::rt::so_environment_params_t()
						.mbox_mutex_pool_size( 4 )
						.agent_event_queue_mutex_pool_size( 4 ) ),
				m_tl1( tl1 ),
				m_tl2( tl2 ),
				m_tl3( tl3 )
		{}

		virtual ~so_environment_t(){}

		virtual void
		init()
		{
			if( nullptr != m_tl1.get() )
				add_extra_layer( std::move( m_tl1 ) );
			if( nullptr != m_tl2.get() )
				add_extra_layer( std::move( m_tl2 ) );
			if( nullptr != m_tl3.get() )
				add_extra_layer( std::move( m_tl3 ) );

			init_finished();
		}

	private:
		std::unique_ptr< test_layer_t< 1 > > m_tl1;
		std::unique_ptr< test_layer_t< 2 > > m_tl2;
		std::unique_ptr< test_layer_t< 3 > > m_tl3;
};

void
check_layers_match(
	test_layer_t< 1 > * tl1,
	test_layer_t< 2 > * tl2,
	test_layer_t< 3 > * tl3,
	const so_environment_t & so_env )
{
	UT_CHECK_EQ(
		so_env.query_layer< test_layer_t< 1 > >( so_5::DO_NOT_THROW_ON_ERROR ),
		tl1 );

	UT_CHECK_EQ(
		so_env.query_layer< test_layer_t< 2 > >( so_5::DO_NOT_THROW_ON_ERROR ),
		tl2 );

	UT_CHECK_EQ(
		so_env.query_layer< test_layer_t< 3 > >( so_5::DO_NOT_THROW_ON_ERROR ),
		tl3 );
}

UT_UNIT_TEST( check_all_exist )
{
	test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( tl1, tl2, tl3 );

	separate_so_thread::start( so_env );

	so_env.wait_for_init_finish();

	check_layers_match( tl1, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_1_2_exist )
{
	test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	test_layer_t< 3 > * tl3 = nullptr;

	so_environment_t so_env( tl1, tl2, tl3 );

	separate_so_thread::start( so_env );

	so_env.wait_for_init_finish();

	check_layers_match( tl1, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_1_3_exist )
{
	test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	test_layer_t< 2 > * tl2 = nullptr;
	test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( tl1, tl2, tl3 );

	separate_so_thread::start( so_env );

	so_env.wait_for_init_finish();

	check_layers_match( tl1, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_2_3_exist )
{
	test_layer_t< 1 > * tl1 = nullptr;
	test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( tl1, tl2, tl3 );

	separate_so_thread::start( so_env );

	so_env.wait_for_init_finish();

	check_layers_match( tl1, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}


#define CHECK_LAYER_DONT_EXISTS( so_env, N ) \
		UT_CHECK_EQ( \
			so_env.query_layer< test_layer_t< N > >( so_5::DO_NOT_THROW_ON_ERROR ), \
			static_cast< so_5::rt::so_layer_t* >( nullptr) )

#define ADD_LAYER( so_env, N ) \
	so_env.add_extra_layer( \
		std::unique_ptr< test_layer_t< N > >( \
			new test_layer_t< N > ) )

#define CHECK_LAYER_EXISTS( so_env, N ) \
		UT_CHECK_EQ( \
			so_env.query_layer< test_layer_t< N > >( so_5::DO_NOT_THROW_ON_ERROR ), \
			last_created_objects[ N ] )


#define CHECK_LAYER( so_env, N ) \
	CHECK_LAYER_DONT_EXISTS( so_env, N ); \
	ADD_LAYER( so_env, N ); \
	CHECK_LAYER_EXISTS( so_env, N );

void
init( so_5::rt::so_environment_t & env )
{
	CHECK_LAYER( env, 1 )
	CHECK_LAYER( env, 2 )
	CHECK_LAYER( env, 3 )
	CHECK_LAYER( env, 4 )
	CHECK_LAYER( env, 5 )
	CHECK_LAYER( env, 6 )
	CHECK_LAYER( env, 7 )
	CHECK_LAYER( env, 8 )
	CHECK_LAYER( env, 9 )
	CHECK_LAYER( env, 10 )
	CHECK_LAYER( env, 11 )
	CHECK_LAYER( env, 12 )
	CHECK_LAYER( env, 13 )
	CHECK_LAYER( env, 14 )
	CHECK_LAYER( env, 15 )
	CHECK_LAYER( env, 16 )
	CHECK_LAYER( env, 17 )
	CHECK_LAYER( env, 18 )
	CHECK_LAYER( env, 19 )
	CHECK_LAYER( env, 20 )
	CHECK_LAYER( env, 21 )
	CHECK_LAYER( env, 22 )
	CHECK_LAYER( env, 23 )
	CHECK_LAYER( env, 24 )
	CHECK_LAYER( env, 25 )
	CHECK_LAYER( env, 26 )
	CHECK_LAYER( env, 27 )
	CHECK_LAYER( env, 28 )
	CHECK_LAYER( env, 29 )
	CHECK_LAYER( env, 30 )
	CHECK_LAYER( env, 31 )
	CHECK_LAYER( env, 32 )
	CHECK_LAYER( env, 33 )
	CHECK_LAYER( env, 34 )
	CHECK_LAYER( env, 35 )
	CHECK_LAYER( env, 36 )
	CHECK_LAYER( env, 37 )
	CHECK_LAYER( env, 38 )
	CHECK_LAYER( env, 39 )
	CHECK_LAYER( env, 40 )
	CHECK_LAYER( env, 41 )
	CHECK_LAYER( env, 42 )
	CHECK_LAYER( env, 43 )
	CHECK_LAYER( env, 44 )
	CHECK_LAYER( env, 45 )
	CHECK_LAYER( env, 46 )
	CHECK_LAYER( env, 47 )
	CHECK_LAYER( env, 48 )
	CHECK_LAYER( env, 49 )
	CHECK_LAYER( env, 50 )
	CHECK_LAYER( env, 51 )
	CHECK_LAYER( env, 52 )
	CHECK_LAYER( env, 53 )
	CHECK_LAYER( env, 54 )
	CHECK_LAYER( env, 55 )
	CHECK_LAYER( env, 56 )
	CHECK_LAYER( env, 57 )
	CHECK_LAYER( env, 58 )
	CHECK_LAYER( env, 59 )
	CHECK_LAYER( env, 60 )
	CHECK_LAYER( env, 61 )
	CHECK_LAYER( env, 62 )
	CHECK_LAYER( env, 63 )

	CHECK_LAYER_EXISTS( env, 1 );
	CHECK_LAYER_EXISTS( env, 2 );
	CHECK_LAYER_EXISTS( env, 3 );
	CHECK_LAYER_EXISTS( env, 4 );
	CHECK_LAYER_EXISTS( env, 5 );
	CHECK_LAYER_EXISTS( env, 6 );
	CHECK_LAYER_EXISTS( env, 7 );
	CHECK_LAYER_EXISTS( env, 8 );
	CHECK_LAYER_EXISTS( env, 9 );
	CHECK_LAYER_EXISTS( env, 10 );
	CHECK_LAYER_EXISTS( env, 11 );
	CHECK_LAYER_EXISTS( env, 12 );
	CHECK_LAYER_EXISTS( env, 13 );
	CHECK_LAYER_EXISTS( env, 14 );
	CHECK_LAYER_EXISTS( env, 15 );
	CHECK_LAYER_EXISTS( env, 16 );
	CHECK_LAYER_EXISTS( env, 17 );
	CHECK_LAYER_EXISTS( env, 18 );
	CHECK_LAYER_EXISTS( env, 19 );
	CHECK_LAYER_EXISTS( env, 20 );
	CHECK_LAYER_EXISTS( env, 21 );
	CHECK_LAYER_EXISTS( env, 22 );
	CHECK_LAYER_EXISTS( env, 23 );
	CHECK_LAYER_EXISTS( env, 24 );
	CHECK_LAYER_EXISTS( env, 25 );
	CHECK_LAYER_EXISTS( env, 26 );
	CHECK_LAYER_EXISTS( env, 27 );
	CHECK_LAYER_EXISTS( env, 28 );
	CHECK_LAYER_EXISTS( env, 29 );
	CHECK_LAYER_EXISTS( env, 30 );
	CHECK_LAYER_EXISTS( env, 31 );
	CHECK_LAYER_EXISTS( env, 32 );
	CHECK_LAYER_EXISTS( env, 33 );
	CHECK_LAYER_EXISTS( env, 34 );
	CHECK_LAYER_EXISTS( env, 35 );
	CHECK_LAYER_EXISTS( env, 36 );
	CHECK_LAYER_EXISTS( env, 37 );
	CHECK_LAYER_EXISTS( env, 38 );
	CHECK_LAYER_EXISTS( env, 39 );
	CHECK_LAYER_EXISTS( env, 40 );
	CHECK_LAYER_EXISTS( env, 41 );
	CHECK_LAYER_EXISTS( env, 42 );
	CHECK_LAYER_EXISTS( env, 43 );
	CHECK_LAYER_EXISTS( env, 44 );
	CHECK_LAYER_EXISTS( env, 45 );
	CHECK_LAYER_EXISTS( env, 46 );
	CHECK_LAYER_EXISTS( env, 47 );
	CHECK_LAYER_EXISTS( env, 48 );
	CHECK_LAYER_EXISTS( env, 49 );
	CHECK_LAYER_EXISTS( env, 50 );
	CHECK_LAYER_EXISTS( env, 51 );
	CHECK_LAYER_EXISTS( env, 52 );
	CHECK_LAYER_EXISTS( env, 53 );
	CHECK_LAYER_EXISTS( env, 54 );
	CHECK_LAYER_EXISTS( env, 55 );
	CHECK_LAYER_EXISTS( env, 56 );
	CHECK_LAYER_EXISTS( env, 57 );
	CHECK_LAYER_EXISTS( env, 58 );
	CHECK_LAYER_EXISTS( env, 59 );
	CHECK_LAYER_EXISTS( env, 60 );
	CHECK_LAYER_EXISTS( env, 61 );
	CHECK_LAYER_EXISTS( env, 62 );
	CHECK_LAYER_EXISTS( env, 63 );

	env.stop();
}



UT_UNIT_TEST( check_many_layers )
{
	UT_CHECK_EQ(
		0,
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t() ) );
}


int
main( int, char ** )
{
	UT_RUN_UNIT_TEST( check_all_exist );
	UT_RUN_UNIT_TEST( check_1_2_exist );
	UT_RUN_UNIT_TEST( check_1_3_exist );
	UT_RUN_UNIT_TEST( check_2_3_exist );
	UT_RUN_UNIT_TEST( check_many_layers );

	return 0;
}
