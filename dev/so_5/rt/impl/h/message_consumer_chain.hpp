/*
	SObjectizer 5.
*/
/*!
	\file
	\brief A class definition for the message consumer chain.
*/
#if !defined( _SO_5__RT__IMPL__MESSAGE_CONSUMER_CHAIN_HPP_ )
#define _SO_5__RT__IMPL__MESSAGE_CONSUMER_CHAIN_HPP_

#include <memory>
#include <map>

#include <so_5/rt/impl/h/message_consumer_link.hpp>
#include <so_5/rt/h/type_wrapper.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// message_consumer_chain_t
//

//! A class for storing a message consumer chain.
class message_consumer_chain_t
{
	public:
		message_consumer_chain_t();
		virtual ~message_consumer_chain_t();

		//! Add consumer to the end of chain.
		void
		push(
			//! Message consumer to be added.
			message_consumer_link_ref_t message_consumer_link );

		//! Remove element from the chain head.
		void
		pop_front();

		//! Remove element from the chain tail.
		void
		pop_back();

		//! Get head of the chain.
		inline message_consumer_link_ref_t &
		query_head()
		{
			return m_front_link;
		}

		//! Is chain empty?
		inline bool
		is_empty() const
		{
			return 0 == m_front_link.get() && 0 == m_back_link.get();
		}

	private:
		//! Chain head.
		message_consumer_link_ref_t m_front_link;
		//! Chain tail.
		message_consumer_link_ref_t m_back_link;
};

//! Typedef for the message_consumer_chain smart pointer.
typedef std::shared_ptr< message_consumer_chain_t >
	message_consumer_chain_ref_t;

//! Typedef for the map from message type to the message_consumer_chain.
typedef std::map<
		type_wrapper_t,
		message_consumer_chain_ref_t >
	msg_type_to_consumer_chain_map_t;

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
