/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for the event exception handler definition.
*/

#if !defined( _SO_5__RT__EVENT_EXCEPTION_HANDLER_HPP_ )
#define _SO_5__RT__EVENT_EXCEPTION_HANDLER_HPP_

#include <memory>
#include <map>
#include <string>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

namespace so_5
{

namespace rt
{

//
// event_exception_response_action_t
//

/*!
 * \brief A base class for actions which should be done
 * after processing of exceptions raised from agent events.
 *
 * Instances of this class will be returned from the 
 * event_exception_handler_t::handle_exception().
 */
class SO_5_TYPE event_exception_response_action_t
{
	public:
		virtual ~event_exception_response_action_t();

		//! Run actions in response to an exception.
		virtual void
		respond_to_exception() = 0;
};

//! Typedef for the event_exception_response_action autopointer.
typedef std::unique_ptr< event_exception_response_action_t >
	event_exception_response_action_unique_ptr_t;

//
// event_exception_handler_t
//

class event_exception_handler_t;

//! Typedef for the event_exception_handler autopointer.
typedef std::unique_ptr< event_exception_handler_t >
	event_exception_handler_unique_ptr_t;

class so_environment_t;

//
// event_exception_handler_t
//

//! An exception handler interface.
/*!
 * An exception handler should handle two actions:
 *
 * 1. Installation of handler into the SObjectizer Environment.
 *    The on_install() method can be reimplemented for this.
 * 2. Caught of an exception and producing corresponding actions.
 *    The handle_exception() method should be reimplemented for this.
 */
class SO_5_TYPE event_exception_handler_t
{
	public:
		virtual ~event_exception_handler_t();

		/*!
		 * \brief A reaction on an exception.
		 * \post Should produce an object with reaction on this exception.
		*/
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! SObjectizer Environment.
			so_environment_t & so_environment,
			//! Exception caught.
			const std::exception & event_exception,
			//! Cooperation name to which agent is belong.
			const std::string & coop_name ) = 0;

		/*!
		 * \brief An installation hook.
		 *
		 * A new exception handler should decide what happened with
		 * the old handler.
		 *
		 * Default implementation simply erases the old handler.
		 */
		virtual void
		on_install(
			//! Old exception handler.
			event_exception_handler_unique_ptr_t previous_handler );
};

//
// create_std_event_exception_handler
//

//! Create a default implementation of the exception handler.
CPP_UTIL_2_EXPORT_FUNC_SPEC( event_exception_handler_unique_ptr_t )
create_std_event_exception_handler();

} /* namespace rt */

} /* namespace so_5 */

#endif

