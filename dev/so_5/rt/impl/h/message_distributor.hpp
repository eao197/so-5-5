/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A class which does distribution for message to consumers.
*/

#if !defined( _SO_5__RT__IMPL__MESSAGE_DISTRIBUTOR_HPP_ )
#define _SO_5__RT__IMPL__MESSAGE_DISTRIBUTOR_HPP_

#include <so_5/rt/impl/h/message_consumer_chain.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//! A class which does distribution for message to consumers.
/*!
 * This class servers two things:
 * - make, store and maintenance of message consumers data;
 * - distribution a message to its consumers.
*/
class message_distributor_t
{
	public:
		message_distributor_t();
		virtual ~message_distributor_t();

		//! Add a first event handler for a consumer.
		void
		push_first(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! Event handler.
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		//! Add another event handler for a consumer.
		ret_code_t
		push_more(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer to me modified.
			impl::message_consumer_link_t * message_consumer_link,
			//! Event handler.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Remove event handler for a consumer.
		ret_code_t
		pop(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer to me modified.
			impl::message_consumer_link_t *
				message_consumer_link,
			//! Event handler to be removed.
			const event_handler_caller_ref_t &
				event_handler_caller_ref,
			//! Receives true if the last event handler was removed.
			bool & is_last_subscription,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Remove all event handlers for a consumer.
		void
		pop(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer to be removed.
			impl::message_consumer_link_t * message_consumer_link );

		//! Deliver message to consumers.
		void
		distribute_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

	private:
		//! Get a message chain for message type given.
		/*!
		 * Creates new chain if there no chain for that message type.
		 */
		message_consumer_chain_t &
		provide_message_consumer_chain(
			//! Message type.
			const type_wrapper_t & type_wrapper );

		//! Map from message types to message consumers.
		msg_type_to_consumer_chain_map_t
			m_msg_type_to_consumer_chain_map;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif

