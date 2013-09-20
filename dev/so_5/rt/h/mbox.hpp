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
#include <so_5/rt/h/message_ref.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_handler_caller_ref.hpp>

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
class agent_ref_t;
class mbox_ref_t;
class message_ref_t;

//
// mbox_t
//

//! Mail box class.
/*!
 * Serves as an interface for sending and receiving messages.
 *
 * All mboxes should created via SObjectizer Environment. References to
 * mboxes are stored and manipulated by so_5::rt::mbox_ref_t objects.
 *
 * mbox_t has two versions of deliver_message() method. First requires
 * pointer to actual message data and intended for delivering messages
 * to agents.
 * Second doesn't not use pointer to actual message data and intended for
 * delivering signals to agents.
 *
 * mbox_t also used for delivery of delayed and periodic messages.
 * SObjectizer Environment stores mbox for which messages should be
 * delivered and timer thread pushes message instances to that mbox
 * at the appropriate time.
 *
 * \see so_environment_t::schedule_timer(), so_environment_t::single_timer().
 */
class SO_5_TYPE mbox_t
	:
		private atomic_refcounted_t
{
		friend class agent_t;
		friend class mbox_ref_t;
		friend class impl::named_local_mbox_t;
		friend class so_5::timer_thread::timer_act_t;

		mbox_t( const mbox_t & );
		void
		operator = ( const mbox_t & );

	public:
		mbox_t();
		virtual ~mbox_t();

		//! Deliver message.
		/*!
		 * Mbox take care about destroying a message object.
		*/
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			std::unique_ptr< MESSAGE > & msg_unique_ptr );


		//! Deliver message.
		/*!
		 * Mbox take care about destroying a message object.
		*/
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			std::unique_ptr< MESSAGE > && msg_unique_ptr );


		//! Deliver signal.
		template< class MESSAGE >
		inline void
		deliver_message();

		//! Get mbox name.
		virtual const std::string &
		query_name() const = 0;

	protected:
		//! Add very first message handler.
		/*!
		 * This method is called when agent is subscribing to message
		 * at first time.
		 */
		virtual void
		subscribe_first_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for that message.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! A very first message handler.
			const event_handler_caller_ref_t &
				event_handler_caller_ref ) = 0;

		//! Add yet another message handler.
		/*!
		 * This method is called when agent is subscribing to message
		 * to which it is already subscribed.
		 */
		virtual ret_code_t
		subscribe_more_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for that message.
			impl::message_consumer_link_t * message_consumer_link,
			//! Message handler for that message.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy ) = 0;

		//! Remove all message handlers.
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for that message.
			impl::message_consumer_link_t *
				message_consumer_link ) = 0;

		//! Deliver message for all subscribers.
		virtual void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref ) = 0;

		//! Get data for object comparision.
		/*!
		 * Default implementation returns this pointer.
		*/
		virtual const mbox_t *
		cmp_ordinal() const;
};


template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > & msg_unique_ptr )
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > && msg_unique_ptr )
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_message()
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t() );
}

} /* namespace rt */

} /* namespace so_5 */

#endif
