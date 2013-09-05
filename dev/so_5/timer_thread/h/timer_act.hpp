/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Timer event definition.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_ACT_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_ACT_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/message_ref.hpp>

namespace so_5
{

namespace timer_thread
{

//
// timer_act_t
//

//! Timer event.
class SO_5_TYPE timer_act_t
{
		timer_act_t( const timer_act_t & );
		void
		operator = ( const timer_act_t & );

	public:
		timer_act_t(
			//! Message type.
			const rt::type_wrapper_t & type_wrapper,
			//! mbox to receive message.
			const rt::mbox_ref_t & mbox,
			//! Message to be sent on timer.
			const rt::message_ref_t & msg,
			/*! Delay for the first message delivery. */
			unsigned int delay,
			/*!
			 * Timeout for periodic messages. Should be 0 for
			 * non-periodic messages.
			 */
			unsigned int period );

		~timer_act_t();

		//! Perform corresponding action on timer.
		/*!
		 * Delivers message to mbox.
		 */
		void
		exec();

		//! Is periodic message?
		bool
		is_periodic() const;

		//! Delay for the first message delivery.
		unsigned int
		query_delay() const;

		/*!
		 * \brief Timeout beetwen deliveries for periodic messages.
		 *
		 * \retval 0  If message is not priodic.
		 */
		unsigned int
		query_period() const;

	private:
		//! Message type.
		const rt::type_wrapper_t m_type_wrapper;

		//! mbox to receive message. 
		rt::mbox_ref_t m_mbox;

		//! Message object to be delivered.
		const rt::message_ref_t m_msg;

		/*! Delay for the first message delivery. */
		const unsigned int m_delay;

		/*!
		 * Timeout for periodic messages. Should be 0 for
		 * non-periodic messages.
		 */
		const unsigned int m_period;
};

//! Auxiliary typedef for timer_act autopointer.
typedef std::unique_ptr< timer_act_t > timer_act_unique_ptr_t;

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

