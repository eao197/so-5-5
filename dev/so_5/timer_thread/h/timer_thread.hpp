/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for the timer_thread.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/timer_thread/h/timer_act.hpp>
#include <so_5/timer_thread/h/timer_id.hpp>

namespace so_5
{

namespace rt
{

} /* namespace rt */

namespace timer_thread
{

//
// timer_thread_t
//

//! Timer thread interface.
/*!

	All timer threads for SObjectizer must be derived from this class.

	A real timer may not be implemented as a thread. The name of this class is
	just a consequence of some historic reasons.

	A timer is started by timer_thread_t::start() method. To stop timer the
	timer_thread_t::finish() method is used. The finish() method should block
	caller until all timer resources will be released and all dedicated
	timer threads (if any) are completelly stopped.

	When so_5::rt::dispatcher_t::push_delayed_msg() is called the dispatcher
	passes timer message to the timer_thread. After this timer is responsible
	for storing and processing this message.
*/
class SO_5_TYPE timer_thread_t
{
	public:

		timer_thread_t();
		virtual ~timer_thread_t();

		//! Launch timer.
		virtual void
		start() = 0;

		//! Finish timer and wait for full stop.
		virtual void
		finish() = 0;

		//! Push delayed/periodic message to the timer queue.
		virtual timer_id_t
		schedule_act(
			timer_act_unique_ptr_t timer_act ) = 0;

		//! Cancel delayer/periodic message.
		virtual void
		cancel_act(
			timer_id_t msg_id ) = 0;
};

//! Auxiliary typedef for timer_thread autopointer.
typedef std::unique_ptr< timer_thread_t > timer_thread_unique_ptr_t;

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
