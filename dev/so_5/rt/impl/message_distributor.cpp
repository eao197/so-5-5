/*
	SObjectizer 5.
*/

#include <so_5/util/h/apply_throwing_strategy.hpp>

#include <so_5/rt/impl/h/message_distributor.hpp>

#include <so_5/rt/h/event_caller_block.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/impl/h/cmp_method_ptr.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

message_distributor_t::message_distributor_t()
{
}

message_distributor_t::~message_distributor_t()
{
}

void
message_distributor_t::push_first(
	const type_wrapper_t & type_wrapper,
	std::unique_ptr< impl::message_consumer_link_t > &
		message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref )
{
	message_consumer_link_ref_t
		ref_ptr( message_consumer_link.release() );

	ref_ptr->event_caller_block()->insert( event_handler_caller_ref );

	provide_message_consumer_chain( type_wrapper )
		.push( ref_ptr );
}

ret_code_t
message_distributor_t::push_more(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t * message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref,
	throwing_strategy_t throwing_strategy )
{
	if( message_consumer_link->event_caller_block()->has(
			event_handler_caller_ref ) )
	{
		return so_5::util::apply_throwing_strategy(
			rc_evt_handler_already_provided,
			throwing_strategy,
			std::string( "duplicating event handler for message " )  +
				type_wrapper.query_type_info().name() );
	}

	event_caller_block_ref_t new_event_caller_block(
		new event_caller_block_t(
			*message_consumer_link->event_caller_block() ) );

	new_event_caller_block->insert( event_handler_caller_ref );

	message_consumer_link->event_caller_block() =
		new_event_caller_block;

	return 0;
}

void
message_distributor_t::pop(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t *
		message_consumer_link )
{
	if( !message_consumer_link->is_first() &&
		!message_consumer_link->is_last() )
	{
		// If this item inside a chain then chain should be modified.
		message_consumer_link->query_right()->set_left(
			message_consumer_link->query_left() );

		message_consumer_link->query_left()->set_right(
			message_consumer_link->query_right() );
	}
	else
	{
		// Special cases for head/tail of chain.
		message_consumer_chain_t &
			message_consumer_chain = provide_message_consumer_chain(
				type_wrapper );

		// Handling of head/tail cases.
		if( message_consumer_link->is_first() )
			message_consumer_chain.pop_front();
		else
			message_consumer_chain.pop_back();

		// Empty chain should be removed.
		if( message_consumer_chain.is_empty() )
			m_msg_type_to_consumer_chain_map.erase(
				type_wrapper );
	}
}

void
message_distributor_t::distribute_message(
	const type_wrapper_t & type_wrapper,
	const message_ref_t & message_ref )
{
	msg_type_to_consumer_chain_map_t::iterator it =
		m_msg_type_to_consumer_chain_map.find(
			type_wrapper );

	// Distribute message only if message consumer chain exists.
	if( m_msg_type_to_consumer_chain_map.end() != it )
	{

		// it->second is message_consumer_chain_ref_t
		message_consumer_link_t * current =
			it->second->query_head().get();

		while( current )
		{
			current->dispatch( message_ref );
			current = current->query_right().get();
		}
	}
}

message_consumer_chain_t &
message_distributor_t::provide_message_consumer_chain(
	const type_wrapper_t & type_wrapper )
{
	msg_type_to_consumer_chain_map_t::iterator it =
		m_msg_type_to_consumer_chain_map.find(
			type_wrapper );

	if( m_msg_type_to_consumer_chain_map.end() != it )
	{
		// Message consumer chain found.
		return *(it->second);
	}

	// New chain should be created.
	message_consumer_chain_ref_t
		new_chain( new message_consumer_chain_t );

	m_msg_type_to_consumer_chain_map.insert(
		msg_type_to_consumer_chain_map_t::value_type(
			type_wrapper,
			new_chain ) );

	return *new_chain;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
