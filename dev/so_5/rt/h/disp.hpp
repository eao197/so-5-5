/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for dispatcher definition.
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

//! Interface for exception handler for dispatcher.
class SO_5_TYPE disp_evt_except_handler_t
{
	public:
		virtual ~disp_evt_except_handler_t();

		//! Handle exception.
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Exception which was caught.
			const std::exception & event_exception,
			//! Cooperation name for agent who threw the exception.
			const std::string & coop_name ) = 0;
};

//
// dispatcher_t
//

//! An interface for all dispatchers.
/*!
 * Dispatcher schedules and calls agents' events.
 *
 * Each agent is binded to dispatcher during registration.
 * A dispatcher_binder_t object is used for that.
 *
 * Each agent stores its events in own event queue. When event is
 * stored in queue agent informs its dispatcher about it. Dispatcher
 * should schedule agent for event execution on agent's working thread
 * context.
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

		//! Launch dispatcher.
		/*!
		 * \return 0 If dispatcher started successfully. Not-null value
		 * indicates that dispatcher cannot be started.
		 */
		virtual ret_code_t
		start() = 0;

		//! Signal about shutdown.
		/*!
		 * Dispatcher should initiate actions for shutting down all
		 * working threads. This method should not block caller until
		 * all threads have beed stopped.
		 */
		virtual void
		shutdown() = 0;

		//! Wait for full stop of dispatcher.
		/*!
		 * This method should block caller until all working thread
		 * have been stopped.
		 */
		virtual void
		wait() = 0;

		//! Schedule execution of agent events.
		virtual void
		put_event_execution_request(
			//! Agent which events should be executed.
			const agent_ref_t & agent_ref,
			//! Count of events for that agent.
			unsigned int event_count ) = 0;

		//! Set exception handler.
		/*!
		 * Caller should take care about 
		 * \a disp_evt_except_handler life-time.
		 */
		void
		set_disp_event_exception_handler(
			disp_evt_except_handler_t & disp_evt_except_handler );

		//! Access to current exception handler.
		disp_evt_except_handler_t &
		query_disp_evt_except_handler();

	private:
		//! Exception handler for the dispatcher.
		disp_evt_except_handler_t * m_disp_evt_except_handler;
};

//! Typedef for dispatcher autopointer.
typedef std::unique_ptr< dispatcher_t > dispatcher_unique_ptr_t;

//! Typedef for dispatcher smart pointer.
typedef std::shared_ptr< dispatcher_t >
	dispatcher_ref_t;

//! Typedef for map from dispatcher name to dispather.
typedef std::map<
		std::string,
		dispatcher_ref_t >
	named_dispatcher_map_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

