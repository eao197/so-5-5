/*
	SObjectizer 5.
*/

#include <iostream>

#include <cpp_util_2/h/lexcast.hpp>

#include <so_5/h/log_err.hpp>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/disp/reuse/work_thread/h/work_thread.hpp>

namespace so_5
{

namespace disp
{

namespace reuse
{

namespace work_thread
{

//
// demand_queue
//
demand_queue_t::demand_queue_t()
{
}

demand_queue_t::~demand_queue_t()
{
	m_demands.clear();
}

void
demand_queue_t::push(
	so_5::rt::agent_t * receiver,
	const so_5::rt::event_caller_block_ref_t & event_caller_block,
	const so_5::rt::message_ref_t & message_ref,
	so_5::rt::demand_handler_pfn_t demand_handler )
{
	std::lock_guard< std::mutex > lock( m_lock );

	if( m_in_service )
	{
		const bool demands_empty_before_service = m_demands.empty();

		m_demands.emplace_back( 
				receiver,
				event_caller_block,
				message_ref,
				demand_handler );

		if( demands_empty_before_service )
		{
			// May be someone is waiting...
			// It should be informed about new demands.
			m_not_empty.notify_one();
		}
	}
}

int
demand_queue_t::pop(
	demand_container_t & demands )
{
	std::unique_lock< std::mutex > lock( m_lock );

	while( true )
	{
		if( m_in_service && !m_demands.empty() )
		{
			demands.swap( m_demands );
			break;
		}
		else if( !m_in_service )
			return shutting_down;
		else
		{
			// Queue is empty. We should wait for a demand or
			// a shutdown signal.
			m_not_empty.wait( lock );
		}
	}

	return demand_extracted;
}

void
demand_queue_t::start_service()
{
	std::lock_guard< std::mutex > lock( m_lock );

	m_in_service = true;
}

void
demand_queue_t::stop_service()
{
	bool is_someone_waiting_us = false;
	{
		std::lock_guard< std::mutex > lock( m_lock );

		m_in_service = false;
		// If the demands queue is empty then someone is waiting
		// for new demands inside pop().
		is_someone_waiting_us = m_demands.empty();
	}

	// In case if someone is waiting.
	if( is_someone_waiting_us )
		m_not_empty.notify_one();
}

void
demand_queue_t::clear()
{
	std::lock_guard< std::mutex > lock( m_lock );
	m_demands.clear();
}

//
// work_thread_t
//

work_thread_t::work_thread_t(
	rt::dispatcher_t & disp )
	:
		m_disp( disp )
{
}

work_thread_t::~work_thread_t()
{
}

void
work_thread_t::start()
{
	m_queue.start_service();
	m_continue_work = WORK_THREAD_CONTINUE;

	m_thread.reset( new std::thread( [this]() { body(); } ) );
}

void
work_thread_t::shutdown()
{
	m_continue_work = WORK_THREAD_STOP;
	m_queue.stop_service();
}

void
work_thread_t::wait()
{
	m_thread->join();

	m_queue.clear();
}

so_5::rt::event_queue_t &
work_thread_t::event_queue()
{
	return m_queue;
}

std::thread::id
work_thread_t::thread_id()
{
	return m_thread->get_id();
}

std::pair< std::thread::id, so_5::rt::event_queue_t * >
work_thread_t::get_agent_binding()
{
	return std::make_pair( thread_id(), &event_queue() );
}

void
work_thread_t::body()
{
	// Local demands queue.
	demand_container_t demands;

	int result = demand_queue_t::no_demands;

	while( m_continue_work == WORK_THREAD_CONTINUE )
	{
		try
		{
			while( m_continue_work == WORK_THREAD_CONTINUE )
			{
				// If the local queue is empty then we should try
				// to get new demands.
				if( demands.empty() )
					result = m_queue.pop( demands );

				// Serve demands if any.
				if( demand_queue_t::demand_extracted == result )
					serve_demands_block( demands );
			}
		}
		catch( const std::exception & x )
		{
			if( !demands.empty() )
				handle_exception( x, *(demands.front().m_receiver) );
			else
				handle_exception_on_empty_demands_queue( x );
		}
	}
}

void
work_thread_t::handle_exception(
	const std::exception & ex,
	so_5::rt::agent_t & a_exception_producer )
{
	log_unhandled_exception( ex, a_exception_producer );

	auto reaction = a_exception_producer.so_exception_reaction();
	if( so_5::rt::abort_on_exception == reaction )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT( "Application will be aborted due to unhandled "
					 	"exception '%s' from cooperation '%s'" ),
				 ex.what(),
				 a_exception_producer.so_coop_name().c_str()) );
		std::abort();
	}
	else if( so_5::rt::shutdown_sobjectizer_on_exception == reaction )
	{
		ACE_ERROR(
				(LM_CRITICAL,
				 SO_5_LOG_FMT( "SObjectizer will be shutted down due to unhandled "
					 	"exception '%s' from cooperation '%s'" ),
				 ex.what(),
				 a_exception_producer.so_coop_name().c_str()) );

		switch_agent_to_special_state_and_shutdown_sobjectizer(
				a_exception_producer );
	}
	else if( so_5::rt::deregister_coop_on_exception == reaction )
	{
		ACE_ERROR(
				(LM_ALERT,
				 SO_5_LOG_FMT( "Cooperation '%s' will be deregistered "
					 	"due to unhandled exception '%s'" ),
				 a_exception_producer.so_coop_name().c_str(),
				 ex.what()) );

		switch_agent_to_special_state_and_deregister_coop(
				a_exception_producer );
	}
	else if( so_5::rt::ignore_exception == reaction )
	{
		ACE_ERROR(
				(LM_WARNING,
				 SO_5_LOG_FMT( "Ignore unhandled exception '%s' from "
					 	"cooperation '%s'"),
				 ex.what(),
				 a_exception_producer.so_coop_name().c_str()) );
	}
	else
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT( "Unknown exception_reaction code: %d. "
					 	"Application will be aborted. "
					 	"Unhandled exception '%s' from cooperation '%s'" ),
				 static_cast< int >(reaction),
				 ex.what(),
				 a_exception_producer.so_coop_name().c_str()) );

		std::abort();
	}
}

void
work_thread_t::handle_exception_on_empty_demands_queue(
	const std::exception & ex )
{
	ACE_ERROR(
			(LM_EMERGENCY,
			 SO_5_LOG_FMT( "An exception caught without the current "
				 	"working agent. Exception: %s" ),
			 ex.what()) );
}

void
work_thread_t::log_unhandled_exception(
	const std::exception & ex_to_log,
	so_5::rt::agent_t & a_exception_producer )
{
	try
	{
		a_exception_producer.so_environment().call_exception_logger(
				ex_to_log,
				a_exception_producer.so_coop_name() );
	}
	catch( const std::exception & x )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT( "An exception '%s' during logging unhandled "
					 	"exception '%s' from cooperation '%s'. "
						"Application will be aborted." ),
				 x.what(),
				 ex_to_log.what(),
				 a_exception_producer.so_coop_name().c_str()) );

		std::abort();
	}
}

void
work_thread_t::switch_agent_to_special_state_and_shutdown_sobjectizer(
	so_5::rt::agent_t & a_exception_producer )
{
	try
	{
		a_exception_producer.so_switch_to_awaiting_deregistration_state();
		a_exception_producer.so_environment().stop();
	}
	catch( const std::exception & x )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT( "An exception '%s' during "
					 	"shutting down SObjectizer on unhandled exception"
						"processing. Application will be aborted." ),
				 x.what()) );

		std::abort();
	}
}

void
work_thread_t::switch_agent_to_special_state_and_deregister_coop(
	so_5::rt::agent_t & a_exception_producer )
{
	const std::string coop_name = a_exception_producer.so_coop_name();
	try
	{
		a_exception_producer.so_switch_to_awaiting_deregistration_state();
		a_exception_producer.so_environment().deregister_coop(
				coop_name,
				so_5::rt::dereg_reason::unhandled_exception );
	}
	catch( const std::exception & x )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT( "An exception '%s' during "
					 	"deregistring cooperation '%s' on unhandled exception"
						"processing. Application will be aborted." ),
				 x.what(),
				 coop_name.c_str()) );

		std::abort();
	}
}

inline void
work_thread_t::serve_demands_block(
	demand_container_t & demands )
{
	while( !demands.empty() )
	{
		demand_t & demand = demands.front();

		(*demand.m_demand_handler)(
				demand.m_message_ref,
				demand.m_event_caller_block.get(),
				demand.m_receiver );

		demands.pop_front();
	}
}

} /* namespace work_thread */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

