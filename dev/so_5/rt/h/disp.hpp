/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for the dispatcher definition.
*/

#if !defined( _SO_5__RT__DISP_HPP_ )
#define _SO_5__RT__DISP_HPP_

#include <memory>
#include <map>
#include <string>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

namespace so_5
{

namespace rt
{

//
// disp_evt_except_handler_t
//

//! Interface of the exception handler for the dispatcher.
class SO_5_TYPE disp_evt_except_handler_t
{
	public:
		virtual ~disp_evt_except_handler_t();

		//! Handle exception.
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Exception which was caught.
			const std::exception & event_exception,
			//! Cooperation name of an agent which threw the exception.
			const std::string & coop_name ) = 0;
};

//
// dispatcher_t
//

//! An interface for all dispatchers.
/*!
 * Dispatcher schedules and calls agents' events.
 *
 * Each agent is binded to a dispatcher during the registration.
 * A dispatcher_binder_t object is used for this.
 *
 * Each agent stores its events in the own event queue. When event is
 * stored in the queue an agent informs its dispatcher about it. 
 * The dispatcher should schedule the agent for the event execution on 
 * the agent's working thread context.
 */
class SO_5_TYPE dispatcher_t
{
	public:
		/*! Do nothing. */
		dispatcher_t();
		/*! Do nothing. */
		virtual ~dispatcher_t();

		/*! Auxiliary method. */
		inline dispatcher_t *
		self_ptr()
		{
			return this;
		}

		//! Launch the dispatcher.
		/*!
		 * \retval 0 if dispatcher has started successfully. 
		 * \retval not-null indicates that the dispatcher cannot be started.
		 */
		virtual ret_code_t
		start() = 0;

		//! Signal about shutdown.
		/*!
		 * Dispatcher must initiate actions for the shutting down of all
		 * working threads. This method shall not block caller until
		 * all threads have beed stopped.
		 */
		virtual void
		shutdown() = 0;

		//! Wait for the full stop of the dispatcher.
		/*!
		 * This method must block the caller until all working threads
		 * have been stopped.
		 */
		virtual void
		wait() = 0;

		//! Schedule execution of agent events.
		virtual void
		put_event_execution_request(
			//! Events of this agent should be executed.
			const agent_ref_t & agent_ref,
			//! Count of events for that agent.
			unsigned int event_count ) = 0;

		//! Set an exception handler.
		/*!
		 * \attention Caller must guarantee that the \a disp_evt_except_handler 
		 * is alive while the dispatcher is in use.
		 */
		void
		set_disp_event_exception_handler(
			disp_evt_except_handler_t & disp_evt_except_handler );

		//! Access to the current exception handler.
		disp_evt_except_handler_t &
		query_disp_evt_except_handler();

	private:
		//! Exception handler for the dispatcher.
		disp_evt_except_handler_t * m_disp_evt_except_handler;
};

//! Typedef of the dispatcher autopointer.
typedef std::unique_ptr< dispatcher_t > dispatcher_unique_ptr_t;

//! Typedef of the dispatcher smart pointer.
typedef std::shared_ptr< dispatcher_t >
	dispatcher_ref_t;

//! Typedef of the map from dispatcher name to a dispather.
typedef std::map<
		std::string,
		dispatcher_ref_t >
	named_dispatcher_map_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

