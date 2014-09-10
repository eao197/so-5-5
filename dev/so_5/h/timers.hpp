/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.5.0
	\brief Timers and tools for working with timers.
*/

#pragma once

#include <chrono>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/message.hpp>

namespace so_5
{

//
// timer_t
//
/*!
 * \since v.5.5.0
 * \brief A base class for timer identificator.
 */
class SO_5_TYPE timer_t
	:	private so_5::rt::atomic_refcounted_t
	{
		friend class so_5::rt::smart_atomic_reference_t< timer_t >;

	public :
		virtual ~timer_t();

		//! Is this timer event is active?
		virtual bool
		is_active() const = 0;

		//! Release the timer event.
		virtual void
		release() = 0;
	};

//
// timer_id_t
//
/*!
 * \since v.5.5.0
 * \brief An indentificator for the timer.
 */
class SO_5_TYPE timer_id_t
	{
	public :
		//! Default constructor.
		timer_id_t();
		//! Initializing constructor.
		timer_id_t(
			so_5::rt::smart_atomic_reference_t< timer_t > && timer );
		//! Copy constructor.
		timer_id_t(
			const timer_id_t & o );
		//! Move constructor.
		timer_id_t(
			timer_id_t && o );
		~timer_id_t();

		//! Copy operator.
		timer_id_t &
		operator=( const timer_id_t & o );
		//! Move operator.
		timer_id_t &
		operator=( timer_id_t && o );

		//! Swapping.
		void
		swap( timer_id_t & o );

		//! Is this timer event is active?
		bool
		is_active() const;

		//! Release the timer event.
		void
		release();

	private :
		//! Actual timer.
		so_5::rt::smart_atomic_reference_t< timer_t > m_timer;
	};

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
*/
class SO_5_TYPE timer_thread_t
	{
		timer_thread_t( const timer_thread_t & ) = delete;
		timer_thread_t &
		operator=( const timer_thread_t & ) = delete;

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
		/*!
		 * A timer can be deactivated later by using returned timer_id.
		 */
		virtual timer_id_t
		schedule(
			//! Type of message to be sheduled.
			const std::type_index & type_index,
			//! Mbox for message delivery.
			so_5::rt::mbox_ref_t mbox,
			//! Message to be sent.
			so_5::rt::message_ref_t msg,
			//! Pause before first message delivery.
			std::chrono::milliseconds pause,
			//! Period for message repetition.
			//! Zero value means single shot delivery.
			std::chrono::milliseconds period ) = 0;

		//! Push anonymous delayed/periodic message to the timer queue.
		/*!
		 * A timer cannot be deactivated later.
		 */
		virtual void
		schedule_anonymous(
			//! Type of message to be sheduled.
			const std::type_index & type_index,
			//! Mbox for message delivery.
			so_5::rt::mbox_ref_t mbox,
			//! Message to be sent.
			so_5::rt::message_ref_t msg,
			//! Pause before first message delivery.
			std::chrono::milliseconds pause,
			//! Period for message repetition.
			//! Zero value means single shot delivery.
			std::chrono::milliseconds period ) = 0;
	};

//! Auxiliary typedef for timer_thread autopointer.
typedef std::unique_ptr< timer_thread_t > timer_thread_unique_ptr_t;

/*!
 * \name Standard timer thread factories.
 * \{
 */
/*!
 * \since v.5.5.0
 * \brief Create timer thread based on timer_wheel mechanism.
 * \note Default parameters will be used for timer thread.
 */
SO_5_EXPORT_FUNC_SPEC( timer_thread_unique_ptr_t )
create_timer_wheel_thread();

/*!
 * \since v.5.5.0
 * \brief Create timer thread based on timer_list mechanism.
 */
SO_5_EXPORT_FUNC_SPEC( timer_thread_unique_ptr_t )
create_timer_list_thread();

/*!
 * \}
 */

} /* namespace so_5 */

