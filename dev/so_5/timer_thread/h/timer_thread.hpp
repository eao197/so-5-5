/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for timer_thread.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/message.hpp>

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

	All timers for SObjectizer should be derived from that class.

	A real timer may not be implemented as a thread. The name of that class is
	just a consequence of some historic reasons.

	A dispatcher starts timer by calling timer_thread_t::start() method.
	Dispatcher informs timer about shutdown by calling
	timer_thread_t::shutdown() method. Sometimes it could be impossible to
	immediately finish timer work. Because of that dispatcher will call
	timer_thread_t::wait() after shutdown().  The dispatcher should be blocked
	on timer_thread_t::wait() call until timer fully finished its work. The
	return from timer_thread_t::wait() means that timer is completelly stopped
	and all resources are released.

	When so_5::rt::dispatcher_t::push_delayed_msg() is called the dispatcher
	passes timer message to the timer_thread. After that timer is responsible
	for storing and processing that message.
*/
class SO_5_TYPE timer_thread_t
{
	public:

		timer_thread_t();
		virtual ~timer_thread_t();

		//! Launch timer.
		virtual ret_code_t
		start() = 0;

		//! Send shutdown signal to timer.
		virtual void
		shutdown() = 0;

		//! Wait timer to stop.
		virtual void
		wait() = 0;

		//! Push delayed/periodic message to timer queue.
		virtual timer_id_t
		schedule_act(
			timer_act_unique_ptr_t & timer_act ) = 0;

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
