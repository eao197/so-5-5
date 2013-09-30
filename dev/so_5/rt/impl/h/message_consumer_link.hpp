/*
	SObjectizer 5.
*/

/*!
	\file
	\brief An interface class for a message consumer chain element.

*/

#if !defined( _SO_5__RT__IMPL__MESSAGE_CONSUMER_LINK_HPP_ )
#define _SO_5__RT__IMPL__MESSAGE_CONSUMER_LINK_HPP_

#include <memory>

#include <so_5/h/types.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class message_consumer_link_t;

//! Typedef for the message_consumer_link autopointer.
typedef std::unique_ptr< message_consumer_link_t >
	message_consumer_link_unique_ptr_t;

//! Typedef for the message_consumer_link smart pointer.
typedef std::shared_ptr< message_consumer_link_t >
	message_consumer_link_ref_t;

//
// message_consumer_link_t
//

//! An interface class for a message consumer chain element.
class message_consumer_link_t
{
	public:
		//! Constructor.
		message_consumer_link_t(
			//! Agent which consumed message.
			const agent_ref_t & agent_ref );

		~message_consumer_link_t();

		//! Dispatch message to the consumer.
		void
		dispatch( const message_ref_t & message );

		//! Set link to left node in the chain.
		void
		set_left(
			const message_consumer_link_ref_t & left );

		//! Set link to right node in the chain.
		void
		set_right(
			const message_consumer_link_ref_t & right );

		//! Get link to left node in the chain.
		inline const message_consumer_link_ref_t &
		query_left() const
		{
			return m_left;
		}

		//! Get link to right node in the chain.
		inline const message_consumer_link_ref_t
		query_right() const
		{
			return m_right;
		}

		//! Is item the head of the chain?
		inline bool
		is_first()
		{
			return 0 == m_left.get();
		}

		//! Is item the tail of the chain?
		inline bool
		is_last()
		{
			return 0 == m_right.get();
		}

		//! Read-only access to the event caller.
		inline const event_caller_block_ref_t &
		event_caller_block() const
		{
			return m_event_handler_caller;
		}

		//! Read-write access to the event caller.
		inline event_caller_block_ref_t &
		event_caller_block()
		{
			return m_event_handler_caller;
		}

	protected:
		//! Left node in the chain.
		message_consumer_link_ref_t m_left;

		//! Right node in the chain.
		message_consumer_link_ref_t m_right;

		//! Event handler caller.
		event_caller_block_ref_t m_event_handler_caller;

		//! Agent-consumer.
		agent_ref_t m_agent_ref;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif

