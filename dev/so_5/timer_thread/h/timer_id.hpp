/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Timer identifier definition.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_ID_REF_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_ID_REF_HPP_

#include <cstdint>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>

namespace so_5
{

namespace timer_thread
{

//! Timer event identifier type.
typedef std::uint_fast64_t timer_id_t;

class timer_thread_t;
class timer_id_ref_t;

//
// timer_id_internals_t
//

/*!
 * \brief A special wrapper on timer_id.
 *
 * Instance destroys timer in the destructor.
 */
class timer_id_internals_t
	:
		private so_5::rt::atomic_refcounted_t
{
		friend class timer_id_ref_t;

		timer_id_internals_t( const timer_id_internals_t & );
		void
		operator = ( const timer_id_internals_t & );

	public:

		timer_id_internals_t(
			//! Timer thread.
			timer_thread_t & timer_thread,
			//! Timer event Id.
			timer_id_t timer_id );

		virtual ~timer_id_internals_t();

	private:
		//! Timer thread.
		timer_thread_t & m_timer_thread;

		//! Timer event Id.
		timer_id_t m_timer_id;
};

//
// timer_id_ref_t
//

//! Smart reference wrapper on timer_id.
class SO_5_TYPE timer_id_ref_t
{
		explicit timer_id_ref_t(
			timer_id_internals_t * timer_id_internals );
	public:

		static timer_id_ref_t
		create(
			//! Timer thread.
			timer_thread_t & timer_thread,
			//! Timer event id.
			timer_id_t timer_id );

		timer_id_ref_t();

		timer_id_ref_t( const timer_id_ref_t & timer_id );
		timer_id_ref_t( timer_id_ref_t && timer_id );

		~timer_id_ref_t();

		timer_id_ref_t &
		operator=( const timer_id_ref_t & timer_id );

		timer_id_ref_t &
		operator=( timer_id_ref_t && timer_id );

		//! Is this timer event is active?
		bool
		is_active() const;

		//! Release the timer event.
		void
		release();

	private:
		//! Increment timer event usage counter.
		void
		inc_timer_id_ref_count();

		//! Decrement timer event usage counter.
		/*!
		 * Destroys timer event if the counter received 0.
		 */
		void
		dec_timer_id_ref_count();

		/*!
		 * \since v.5.2.0
		 * \brief A part of move ownership of timer_id to another object.
		 */
		timer_id_internals_t *
		release_ownership();

		//! Timer event identification data.
		timer_id_internals_t * m_timer_id_internals;
};

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

