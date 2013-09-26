/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Mbox definition.
*/


#if !defined( _SO_5__RT__MBOX_HPP_ )
#define _SO_5__RT__MBOX_HPP_

#include <string>
#include <memory>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/h/declspec.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_handler_caller_ref_fwd.hpp>

namespace so_5
{

namespace timer_thread
{
	class timer_act_t;
}

namespace rt
{

namespace impl
{

class message_consumer_link_t;
class named_local_mbox_t;

} /* namespace impl */

class agent_t;

//
// mbox_t
//

//! Mail box class.
/*!
 * The class serves as an interface for sending and receiving messages.
 *
 * All mboxes can be created via the SObjectizer Environment. References to
 * mboxes are stored and manipulated by so_5::rt::mbox_ref_t objects.
 *
 * mbox_t has two versions of the deliver_message() method. 
 * The first one requires pointer to the actual message data and is intended 
 * for delivering messages to agents.
 * The second one doesn't use a pointer to the actual message data and 
 * is intended for delivering signals to agents.
 *
 * mbox_t also is used for the delivery of delayed and periodic messages.
 * The SObjectizer Environment stores mbox for which messages must be
 * delivered and the timer thread pushes message instances to the mbox
 * at the appropriate time.
 *
 * \see so_environment_t::schedule_timer(), so_environment_t::single_timer().
 */
class SO_5_TYPE mbox_t
	:
		private atomic_refcounted_t
{
		friend class agent_t;
		friend class impl::named_local_mbox_t;
		friend class so_5::timer_thread::timer_act_t;

		friend class smart_atomic_reference_t< mbox_t >;

		mbox_t( const mbox_t & );
		void
		operator = ( const mbox_t & );

	public:
		mbox_t();
		virtual ~mbox_t();

		//! Deliver message.
		/*!
		 * Mbox takes care about destroying a message object.
		*/
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			std::unique_ptr< MESSAGE > msg_unique_ptr );


		//! Deliver signal.
		template< class MESSAGE >
		inline void
		deliver_signal();

		//! Get the mbox name.
		virtual const std::string &
		query_name() const = 0;

		/*!
		 * \name Comparision.
		 * \{
		 */
		bool operator==( const mbox_t & o ) const;

		bool operator<( const mbox_t & o ) const;
		/*!
		 * \}
		 */

	protected:
		//! Add the first message handler.
		/*!
		 * This method is called when the agent is subscribing to the message
		 * at the first time.
		 */
		virtual void
		subscribe_first_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for this message.
			std::unique_ptr< impl::message_consumer_link_t >
				message_consumer_link,
			//! The very first message handler.
			const event_handler_caller_ref_t &
				event_handler_caller_ref ) = 0;

		//! Add yet another message handler.
		/*!
		 * This method is called when the agent is subscribing to the message
		 * which is already subscribed.
		 */
		virtual ret_code_t
		subscribe_more_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for this message.
			impl::message_consumer_link_t * message_consumer_link,
			//! Message handler for this message.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy ) = 0;

		//! Remove all message handlers.
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for this message.
			impl::message_consumer_link_t *
				message_consumer_link ) = 0;

		//! Deliver message for all subscribers.
		virtual void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref ) = 0;

		//! Get data for the object comparision.
		/*!
		 * Default implementation returns this pointer.
		*/
		virtual const mbox_t *
		cmp_ordinal() const;
};

template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > msg_unique_ptr )
{
	ensure_message_with_actual_data( msg_unique_ptr.get() );

	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_signal()
{
	ensure_signal< MESSAGE >();

	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t() );
}

//
// mbox_ref_t
//
//! Smart reference for the mbox_t.
/*!
 * \note Defined as typedef since v.5.2.0
 */
typedef smart_atomic_reference_t< mbox_t > mbox_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
