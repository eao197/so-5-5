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
	if( message_consumer_link->event_caller_block()
		->has( event_handler_caller_ref ) )
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


ret_code_t
message_distributor_t::pop(
	const type_wrapper_t & type_wrapper,
	message_consumer_link_t * message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref,
	bool & is_last_subscription,
	throwing_strategy_t throwing_strategy )
{
	event_caller_block_ref_t new_event_caller_block(
		new event_caller_block_t(
			*message_consumer_link->event_caller_block() ) );

	const event_handler_caller_t * ehc =
		new_event_caller_block->find( event_handler_caller_ref );

	if( ehc )
	{
		// ≈сли это тот самый метод который был подписан,
		// то будем удал€ть.
		if( impl::cmp_method_ptr(
				ehc->ordinal(),
				ehc->ordinal_size(),
				event_handler_caller_ref->ordinal(),
				event_handler_caller_ref->ordinal_size() ) )
		{
			new_event_caller_block->erase(  event_handler_caller_ref );
		}
		else
		{
			return so_5::util::apply_throwing_strategy(
				rc_event_handler_match_error,
				throwing_strategy,
				std::string( "method doesn't match subscribed one for message " ) +
				type_wrapper.query_type_info().name() );
		}
	}
	else
	{
		return so_5::util::apply_throwing_strategy(
			rc_no_event_handler_provided,
			throwing_strategy,
			std::string( "no event handler provided for message " ) +
			type_wrapper.query_type_info().name() );
	}

	message_consumer_link->event_caller_block() =
		new_event_caller_block;

	is_last_subscription = new_event_caller_block->is_empty();

	if( is_last_subscription )
	{
		pop( type_wrapper, message_consumer_link );
	}

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
		// ≈сли это внутренне звено цепи,
		// то саму цепочку искать не надо,
		// а надо только извлечь звено.
		message_consumer_link->query_right()->set_left(
			message_consumer_link->query_left() );

		message_consumer_link->query_left()->set_right(
			message_consumer_link->query_right() );
	}
	else
	{
		// ≈сли имеем дело с началом либо концом цепочки,
		// то надо доставать саму цепь.
		message_consumer_chain_t &
			message_consumer_chain = provide_message_consumer_chain(
				type_wrapper );

		// »звлекаем либо начало либо конец.
		if( message_consumer_link->is_first() )
			message_consumer_chain.pop_front();
		else
			message_consumer_chain.pop_back();

		// ≈сли цепочка выродилась, то удал€ем ее.
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

	// ≈сли така€ цепочка есть.
	if( m_msg_type_to_consumer_chain_map.end() != it )
	{

		// it->second - это message_consumer_chain_ref_t
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
		// ≈сли така€ цепочка есть, то вернем ее.
		return *(it->second);
	}

	// ≈сли такой цепочки еще не существует, то создаем
	// и возвращаем ее.
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
