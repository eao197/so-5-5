/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of utility class for work with dispatchers.
*/

#if !defined( _SO_5__RT__IMPL__DISP_CORE_HPP_ )
#define _SO_5__RT__IMPL__DISP_CORE_HPP_

#include <ace/Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/rt/h/disp.hpp>

#include <so_5/rt/h/event_exception_logger.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// disp_core_t
//

//! A utility class for work with dispatchers.
class disp_core_t
	:
		public disp_evt_except_handler_t
{
	public:
		disp_core_t(
			so_environment_t & so_environment,
			const named_dispatcher_map_t & named_dispatcher_map,
			event_exception_logger_unique_ptr_t logger,
			event_exception_handler_unique_ptr_t handler );

		virtual ~disp_core_t();

		//! Get the default dispatcher.
		dispatcher_t &
		query_default_dispatcher();

		//! Get a named dispatcher.
		/*!
			\return null reference if dispatcher with \a disp_name
			has not been found.
		*/
		dispatcher_ref_t
		query_named_dispatcher(
			const std::string & disp_name );

		//! Start all dispatchers.
		ret_code_t
		start();

		//! Send shutdown signal for all dispatchers.
		void
		shutdown();

		//! Blocking wait for complete shutdown of all dispatchers.
		void
		wait();

		//! Install exception logger.
		void
		install_exception_logger(
			event_exception_logger_unique_ptr_t logger );

		//! Install exception handler.
		void
		install_exception_handler(
			event_exception_handler_unique_ptr_t handler );

		//! Handle an exception.
		/*!
		 * Calls exception logger and handler.
		*/
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Exception caught.
			const std::exception & event_exception,
			//! A cooperation to which agent is belong.
			const std::string & coop_name );

	private:
		//! SObjectizer Environment to work with.
		so_environment_t & m_so_environment;

		//! Default dispatcher.
		dispatcher_unique_ptr_t m_default_dispatcher;

		//! Named dispatchers.
		named_dispatcher_map_t m_named_dispatcher_map;

		//! Exception logger object lock.
		ACE_Thread_Mutex m_exception_logger_lock;

		//! Exception logger.
		event_exception_logger_unique_ptr_t m_event_exception_logger;

		//! Exception handler.
		event_exception_handler_unique_ptr_t m_event_exception_handler;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
