/*
 * This file should be #included into test source file.
 */
#include <ace/OS.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Guard_T.h>

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

template< class ENV, class FUNCTOR >
void
run_on( ENV & env, FUNCTOR f )
{
	start( env );
	env.wait_for_init_finish();

	f();

	env.stop();
	wait( env );
}

} /* namespace separate_so_thread */

