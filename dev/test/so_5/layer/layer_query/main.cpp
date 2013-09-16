/*
 * A test for layers set and get.
 */

#include <iostream>
#include <map>
#include <exception>

#include <ace/OS.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
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

} /* namespace separate_so_thread */

void * last_created_objects[ 64 ];

template < int N >
class test_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:
		test_layer_t()
		{
			last_created_objects[ N ] = this;
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
	:
		public so_5::rt::so_environment_t
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
						.agent_event_queue_mutex_pool_size( 4 )
						.add_layer(
							std::unique_ptr< test_layer_t< 1 > >( tl1 ) )
						.add_layer(
							std::unique_ptr< test_layer_t< 2 > >( tl2 ) )
						.add_layer(
							std::unique_ptr< test_layer_t< 3 > >( tl3 ) ) )
		{}

		virtual ~so_environment_t(){}

		virtual void
		init()
		{
			stop();
		}
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

	separate_so_thread::start(
		so_env );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000 ) );

	check_layers_match( tl1, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_1_2_exist )
{
	test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	// test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( tl1, tl2, 0 );

	separate_so_thread::start(
		so_env );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000 ) );

	check_layers_match( tl1, tl2, 0, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_1_3_exist )
{
	test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	// test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( tl1, 0, tl3 );

	separate_so_thread::start(
		so_env );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000 ) );

	check_layers_match( tl1, 0, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

UT_UNIT_TEST( check_2_3_exist )
{
	//test_layer_t< 1 > * tl1 = new test_layer_t< 1 >;
	test_layer_t< 2 > * tl2 = new test_layer_t< 2 >;
	test_layer_t< 3 > * tl3 = new test_layer_t< 3 >;

	so_environment_t so_env( 0, tl2, tl3 );

	separate_so_thread::start(
		so_env );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000 ) );

	check_layers_match( 0, tl2, tl3, so_env );

	so_env.stop();

	separate_so_thread::wait(
		so_env );
}

#define CHECK_LAYER_EXISTANCE( so_env, N ) \
		UT_CHECK_EQ( \
			so_env.query_layer< test_layer_t< N > >(), \
			(test_layer_t< N >*) last_created_objects[ N ] )

void
init( so_5::rt::so_environment_t & env )
{
	CHECK_LAYER_EXISTANCE( env, 1 );
	CHECK_LAYER_EXISTANCE( env, 2 );
	CHECK_LAYER_EXISTANCE( env, 3 );
	CHECK_LAYER_EXISTANCE( env, 4 );
	CHECK_LAYER_EXISTANCE( env, 5 );
	CHECK_LAYER_EXISTANCE( env, 6 );
	CHECK_LAYER_EXISTANCE( env, 7 );
	CHECK_LAYER_EXISTANCE( env, 8 );
	CHECK_LAYER_EXISTANCE( env, 9 );
	CHECK_LAYER_EXISTANCE( env, 10 );
	CHECK_LAYER_EXISTANCE( env, 11 );
	CHECK_LAYER_EXISTANCE( env, 12 );
	CHECK_LAYER_EXISTANCE( env, 13 );
	CHECK_LAYER_EXISTANCE( env, 14 );
	CHECK_LAYER_EXISTANCE( env, 15 );
	CHECK_LAYER_EXISTANCE( env, 16 );
	CHECK_LAYER_EXISTANCE( env, 17 );
	CHECK_LAYER_EXISTANCE( env, 18 );
	CHECK_LAYER_EXISTANCE( env, 19 );
	CHECK_LAYER_EXISTANCE( env, 20 );
	CHECK_LAYER_EXISTANCE( env, 21 );
	CHECK_LAYER_EXISTANCE( env, 22 );
	CHECK_LAYER_EXISTANCE( env, 23 );
	CHECK_LAYER_EXISTANCE( env, 24 );
	CHECK_LAYER_EXISTANCE( env, 25 );
	CHECK_LAYER_EXISTANCE( env, 26 );
	CHECK_LAYER_EXISTANCE( env, 27 );
	CHECK_LAYER_EXISTANCE( env, 28 );
	CHECK_LAYER_EXISTANCE( env, 29 );
	CHECK_LAYER_EXISTANCE( env, 30 );
	CHECK_LAYER_EXISTANCE( env, 31 );
	CHECK_LAYER_EXISTANCE( env, 32 );

	env.stop();
}


#define ADD_LAYER( N ) \
	.add_layer( \
		std::unique_ptr< test_layer_t< N > >( \
			new test_layer_t< N > ) )

UT_UNIT_TEST( check_many_layers )
{
	UT_CHECK_EQ(
		0,
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
				ADD_LAYER( 1 )
				ADD_LAYER( 2 )
				ADD_LAYER( 3 )
				ADD_LAYER( 4 )
				ADD_LAYER( 5 )
				ADD_LAYER( 6 )
				ADD_LAYER( 7 )
				ADD_LAYER( 8 )
				ADD_LAYER( 9 )
				ADD_LAYER( 10 )
				ADD_LAYER( 11 )
				ADD_LAYER( 12 )
				ADD_LAYER( 13 )
				ADD_LAYER( 14 )
				ADD_LAYER( 15 )
				ADD_LAYER( 16 )
				ADD_LAYER( 17 )
				ADD_LAYER( 18 )
				ADD_LAYER( 19 )
				ADD_LAYER( 20 )
				ADD_LAYER( 21 )
				ADD_LAYER( 22 )
				ADD_LAYER( 23 )
				ADD_LAYER( 24 )
				ADD_LAYER( 25 )
				ADD_LAYER( 26 )
				ADD_LAYER( 27 )
				ADD_LAYER( 28 )
				ADD_LAYER( 29 )
				ADD_LAYER( 30 )
				ADD_LAYER( 31 )
				ADD_LAYER( 32 )
			) );
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
