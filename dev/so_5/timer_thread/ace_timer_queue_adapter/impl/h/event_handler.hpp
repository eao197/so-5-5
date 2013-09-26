/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Timer event handler.
 */

#if !defined( _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__EVENT_HANDLER_HPP_ )
#define _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__EVENT_HANDLER_HPP_

#include <ace/Event_Handler.h>

#include <so_5/timer_thread/h/timer_act.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

namespace impl
{

//
// timer_act_utilizator_t
//

//! An interface for the timer event destroyer.
class timer_act_utilizator_t
{
	public:
		timer_act_utilizator_t(){}
		virtual ~timer_act_utilizator_t(){}

		//! Destroy timer event.
		virtual void
		utilize_timer_act( timer_act_t * timer_act ) = 0;
};


//
// timer_event_handler_t
//

/*!
	\brief Timer event handler.

	An instance of this class is registered as a timer handler in
	the ACE_Thread_Timer_Queue_Adapter.
*/

class timer_event_handler_t
	:
		public ACE_Event_Handler
{
	public:
		//! Constructor.
		timer_event_handler_t(
				timer_act_utilizator_t & utilizator );
		virtual ~timer_event_handler_t();

		//! Timer handler routine.
		/*!
			\return 0
		 */
		virtual int
		handle_timeout(
				const ACE_Time_Value &,
				const void * raw_act );

	private:
		//! An object to destroy the timer event.
		timer_act_utilizator_t & m_utilizator;
};

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

