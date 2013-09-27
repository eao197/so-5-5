/*
	SObjectizer 5.
*/

#include <ace/Guard_T.h>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/agent_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

agent_core_t::agent_core_t(
	so_environment_t & so_environment,
	unsigned int agent_coop_mutex_pool_size,
	unsigned int agent_queue_mutex_pool_size,
	coop_listener_unique_ptr_t coop_listener )
	:
		m_so_environment( so_environment ),
		m_agent_coop_mutex_pool( agent_coop_mutex_pool_size ),
		m_agent_queue_mutex_pool( agent_queue_mutex_pool_size ),
		m_deregistration_started_cond( m_coop_operations_lock ),
		m_deregistration_finished_cond( m_coop_operations_lock ),
		m_deregistration_started( false ),
		m_coop_listener( std::move( coop_listener ) )
{
}

agent_core_t::~agent_core_t()
{
}

void
agent_core_t::start()
{
	m_deregistration_started = false;
	m_coop_dereg_executor.start();
}

void
agent_core_t::shutdown()
{
	// Deregistration of all cooperations should be initiated.
	deregister_all_coop();

	// Deregistration of all cooperations should be finished.
	wait_all_coop_to_deregister();

	// Notify a dedicated thread.
	m_coop_dereg_executor.shutdown();
}

void
agent_core_t::wait()
{
	// The dedicated thread should be stopped.
	m_coop_dereg_executor.wait();
}

ACE_Thread_Mutex &
agent_core_t::allocate_agent_coop_mutex()
{
	return m_agent_coop_mutex_pool.allocate_mutex();
}

void
agent_core_t::deallocate_agent_coop_mutex(
	ACE_Thread_Mutex & m )
{
	m_agent_coop_mutex_pool.deallocate_mutex( m );
}

local_event_queue_unique_ptr_t
agent_core_t::create_local_queue()
{
	local_event_queue_unique_ptr_t local_event_queue(
		new local_event_queue_t( m_agent_queue_mutex_pool ) );

	return local_event_queue;
}

void
agent_core_t::register_coop(
	agent_coop_unique_ptr_t agent_coop )
{
	if( 0 == agent_coop.get() )
		SO_5_THROW_EXCEPTION(
			rc_zero_ptr_to_coop,
			"zero ptr to coop passed" );

	const std::string & coop_name = agent_coop->query_coop_name();

	try
	{
		agent_coop->bind_agents_to_coop();

		// All the following actions should be taken under the lock.
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		// Name should be unique.
		if( m_registered_coop.end() !=
			m_registered_coop.find( coop_name ) ||
			m_deregistered_coop.end() !=
			m_deregistered_coop.find( coop_name ) )
		{
			SO_5_THROW_EXCEPTION(
				rc_coop_with_specified_name_is_already_registered,
				"coop with name \"" + coop_name + "\" is already registered" );
		}

		agent_coop->define_all_agents();
		agent_coop->bind_agents_to_disp();

		m_registered_coop[ coop_name ] =
			agent_coop_ref_t( agent_coop.release() );
	}
	catch( const exception_t & ex )
	{
		throw;
	}
	catch( const std::exception & ex )
	{
		SO_5_THROW_EXCEPTION(
			rc_coop_define_agent_failed,
			ex.what() );
	}

	if( m_coop_listener.get() )
		m_coop_listener->on_registered( m_so_environment, coop_name );
}

void
agent_core_t::deregister_coop(
	const nonempty_name_t & name )
{
	const std::string & coop_name = name.query_name();

	agent_coop_ref_t coop;

	{
		// All the following actions should be taken under the lock.
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		// No action if cooperation is already in deregistration process.
		if( m_deregistered_coop.end() !=
			m_deregistered_coop.find( coop_name ) )
		{
			return;
		}

		// It is an error if cooperation is not registered.
		coop_map_t::iterator it = m_registered_coop.find( coop_name );
		if( m_registered_coop.end() == it )
		{
			SO_5_THROW_EXCEPTION(
				rc_coop_has_not_found_among_registered_coop,
				"coop with name \"" + coop_name +
				"\" not found among registered cooperations" );
		}

		coop = it->second;

		m_registered_coop.erase( it );

		m_deregistered_coop[ coop_name ] = coop;
	}

	// All agents of cooperation should be marked as deregistered.
	coop->undefine_all_agents();
}

void
agent_core_t::ready_to_deregister_notify(
	agent_coop_t * coop )
{
	m_coop_dereg_executor.push_dereg_demand( coop );
}

void
agent_core_t::final_deregister_coop(
	const std::string coop_name )
{
	bool need_signal_dereg_finished;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		m_deregistered_coop.erase( coop_name );

		// If we are inside shutdown process and this is the last
		// cooperation then special flag should be set.
		need_signal_dereg_finished =
			m_deregistration_started && m_deregistered_coop.empty();
	}

	if( need_signal_dereg_finished )
		m_deregistration_finished_cond.signal();

	if( m_coop_listener.get() )
		m_coop_listener->on_deregistered(
			m_so_environment,
			coop_name );
}

void
agent_core_t::start_deregistration()
{
	bool signal_deregistration_started = false;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		if( !m_deregistration_started )
		{
			m_deregistration_started = true;
			signal_deregistration_started = true;
		}
	}

	if( signal_deregistration_started )
		m_deregistration_started_cond.signal();
}

void
agent_core_t::wait_for_start_deregistration()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	if( !m_deregistration_started )
		m_deregistration_started_cond.wait();
}

void
agent_core_t::coop_undefine_all_agents(
	agent_core_t::coop_map_t::value_type & coop )
{
	coop.second->undefine_all_agents();
}

void
agent_core_t::deregister_all_coop()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	std::for_each(
		m_registered_coop.begin(),
		m_registered_coop.end(),
		agent_core_t::coop_undefine_all_agents );

	m_deregistered_coop.insert(
		m_registered_coop.begin(),
		m_registered_coop.end() );

	m_registered_coop.clear();
	m_deregistration_started = true;
}

void
agent_core_t::wait_all_coop_to_deregister()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	// Must wait for a signal is there are cooperations in
	// deregistration process.
	if( !m_deregistered_coop.empty() )
	{
		// Wait for deregistration finish.
		m_deregistration_finished_cond.wait();
	}
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
