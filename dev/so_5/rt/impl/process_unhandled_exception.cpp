/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \file
 * \brief Helpers for handling unhandled exceptions from agent's event handlers.
 */

#include <so_5/rt/impl/h/process_unhandled_exception.hpp>

#include <cstdlib>

#include <so_5/h/log_err.hpp>

#include <so_5/rt/h/so_environment.hpp>

namespace so_5 {

namespace rt {

namespace impl {

namespace {

/*!
 * \since v.5.4.0
 * \brief Switch agent to special state and deregister its cooperation.
 *
 * Calls abort() if an exception is raised during work.
 */
void
switch_agent_to_special_state_and_deregister_coop(
	//! Agent who is the producer of the exception.
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

/*!
 * \since v.5.4.0
 * \brief Switch agent to special state and initiate stopping
 * of SObjectizer Environment.
 *
 * Calls abort() if an exception is raised during work.
 */
void
switch_agent_to_special_state_and_shutdown_sobjectizer(
	//! Agent who is the producer of the exception.
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

/*!
 * \since v.5.4.0
 * \brief Log unhandled exception from cooperation.
 *
 * Calls abort() if an exception is raised during logging.
 */
void
log_unhandled_exception(
	//! Raised and caught exception.
	const std::exception & ex_to_log,
	//! Agent who is the producer of the exception.
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

} /* namespace anonymous */

//
// process_unhandled_exception
//
void
process_unhandled_exception(
	const std::exception & ex,
	so_5::rt::agent_t & a_exception_producer )
	{
		log_unhandled_exception( ex, a_exception_producer );

//FIXME: there must be handling of special case for
//checking working_thread_id and exception_reaction.

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
					 SO_5_LOG_FMT( "SObjectizer will be shutted down due to "
						 	"unhandled exception '%s' from cooperation '%s'" ),
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

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

