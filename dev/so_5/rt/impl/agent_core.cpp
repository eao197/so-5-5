/*
	SObjectizer 5.
*/

#include <ace/Guard_T.h>

#include <so_5/util/h/apply_throwing_strategy.hpp>

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
	unsigned int agent_coop_mutex_pool,
	unsigned int agent_queue_mutex_pool_size,
	coop_listener_unique_ptr_t coop_listener )
	:
		m_so_environment( so_environment ),
		m_agent_coop_mutex_pool( agent_coop_mutex_pool ),
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
	// »нициируем дерегистрацию всех коопераций.
	deregister_all_coop();

	// ∆дем дерегистрации всех коопераций.
	wait_all_coop_to_deregister();

	// «авершаем нить дерегистратора агентов.
	m_coop_dereg_executor.shutdown();
}

void
agent_core_t::wait()
{
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

ret_code_t
agent_core_t::register_coop(
	agent_coop_unique_ptr_t agent_coop,
	throwing_strategy_t throwing_strategy )
{
	if( 0 == agent_coop.get() )
	{
		return so_5::util::apply_throwing_strategy(
			rc_zero_ptr_to_coop,
			throwing_strategy,
			"zero ptr to coop passed" );
	}

	const std::string & coop_name = agent_coop->query_coop_name();

	try
	{

		agent_coop->bind_agents_to_coop();

		// ƒалее необходим замок.
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		// ѕровер€ем нет ли такого имени в данный момент.
		if( m_registered_coop.end() !=
			m_registered_coop.find( coop_name ) ||
			m_deregistered_coop.end() !=
			m_deregistered_coop.find( coop_name ) )
		{
			return so_5::util::apply_throwing_strategy(
				rc_coop_with_specified_name_is_already_registered,
				throwing_strategy,
				"coop with name \"" + coop_name +
				"\" is already registered" );
		}

		agent_coop->define_all_agents();
		agent_coop->bind_agents_to_disp();

		m_registered_coop[ coop_name ] =
			agent_coop_ref_t( agent_coop.release() );
	}
	catch( const exception_t & ex )
	{
		return so_5::util::apply_throwing_strategy(
			ex,
			throwing_strategy );
	}
	catch( const std::exception & ex )
	{
		return so_5::util::apply_throwing_strategy(
			rc_coop_define_agent_failed,
			throwing_strategy,
			ex.what() );
	}

	if( m_coop_listener.get() )
		m_coop_listener->on_registered(
			m_so_environment,
			coop_name );

	return 0;
}

ret_code_t
agent_core_t::deregister_coop(
	const nonempty_name_t & name,
	throwing_strategy_t throwing_strategy )
{
	const std::string & coop_name = name.query_name();

	agent_coop_ref_t coop;

	{
		// Ќужен замок.
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		// ≈сли коопераци€ и так среди дерегистрированных, то выходим.
		if( m_deregistered_coop.end() !=
			m_deregistered_coop.find( coop_name ) )
		{
			return 0;
		}

		// ≈сли кооперации нет среди зарегистрированных, то это ошибка.
		coop_map_t::iterator it = m_registered_coop.find( coop_name );
		if( m_registered_coop.end() == it )
		{
			return so_5::util::apply_throwing_strategy(
				rc_coop_has_not_found_among_registered_coop,
				throwing_strategy,
				"coop with name \"" + coop_name +
				"\" not found among registered cooperations" );
		}

		// –аз така€ коопераци€ есть, то перенесем ее в список
		// дерегистрируемых коопераций.
		coop = it->second;

		//”дал€ем из зарегистрированных.
		m_registered_coop.erase( it );

		// ѕереносим кооперацию в список дерегистрируемых.
		m_deregistered_coop[ coop_name ] = coop;
	}

	// ќтмечаем всех агентов как дерегистрируемых.
	coop->undefine_all_agents();

	return 0;
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

		// ≈сли мы находимс€ в режиме завершени€ работы,
		// когда должны дерегистрироватьс€ все кооперации,
		// тогда в случае если это последн€€ коопераци€,
		// надо просигналить что это последн€€ коопераци€.
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

	// ≈сли дерегистраци€ еще не началась, то
	// ожидаем сигнала о ее начале.
	if( !m_deregistration_started )
		m_deregistration_started_cond.wait();
}

void
agent_core_t::coop_undefine_all_agents(
	agent_core_t::coop_map_t::value_type & coop )
{
	// ќтмечаем всех агентов как дерегистрируемых.
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

	// NOTE: мы здесь не отсылаем сигнал о том
	// что m_deregistration_finished в случае если m_deregistered_coop
	// пуст, потому что deregister_all_coop() и wait_all_coop_to_deregister()
	// вызываютс€ на одной нити друг за другом, а значит если
	// коопераций никаких и нет, то wait_all_coop_to_deregister()
	// и не будет ожидать событи€.
}

void
agent_core_t::wait_all_coop_to_deregister()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	// ≈сли есть не до конца дерегистрированные кооперации,
	// то будем ожидать сигнала.
	if( !m_deregistered_coop.empty() )
	{
		// ќжидаем сигнала.
		m_deregistration_finished_cond.wait();
	}
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
