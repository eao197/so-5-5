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
#include <typeindex>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/message.hpp>

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
			const std::type_index & type_index,
			//! mbox to receive a message.
			const rt::mbox_ref_t & mbox,
			//! Message to be sent on timer.
			const rt::message_ref_t & msg,
			/*! Delay for the first message delivery. */
			unsigned int delay,
			/*!
			 * Timeout for periodic messages. Shall be 0 for
			 * non-periodic messages.
			 */
			unsigned int period );

		~timer_act_t();

		//! Perform corresponding action on timer.
		/*!
		 * Delivers message to the mbox.
		 */
		void
		exec();

		//! It's a periodic message?
		bool
		is_periodic() const;

		//! Delay for the first message delivery.
		unsigned int
		query_delay() const;

		/*!
		 * \brief Timeout between deliveries for periodic messages.
		 *
		 * \retval 0 If the message is not periodic.
		 */
		unsigned int
		query_period() const;

	private:
		//! Message type.
		const std::type_index m_type_index;

		//! mbox to receive a message. 
		rt::mbox_ref_t m_mbox;

		//! Message object to be delivered.
		const rt::message_ref_t m_msg;

		/*! Delay for the first message delivery. */
		const unsigned int m_delay;

		/*!
		 * Timeout for periodic messages. Must be 0 for
		 * non-periodic messages.
		 */
		const unsigned int m_period;
};

//! Auxiliary typedef for the timer_act autopointer.
typedef std::unique_ptr< timer_act_t > timer_act_unique_ptr_t;

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

