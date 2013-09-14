/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/rt/impl/h/named_local_mbox.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// named_local_mbox_t
//

named_local_mbox_t::named_local_mbox_t(
	const std::string & name,
	const mbox_ref_t & mbox,
	impl::mbox_core_t & mbox_core )
	:
		m_name( name ),
		m_mbox_core( &mbox_core ),
		m_mbox( mbox )
{
}

named_local_mbox_t::~named_local_mbox_t()
{
	m_mbox_core->destroy_mbox( m_name );
}

void
named_local_mbox_t::subscribe_first_event_handler(
	const type_wrapper_t & type_wrapper,
	std::unique_ptr< impl::message_consumer_link_t > &
		message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref )
{
	m_mbox->subscribe_first_event_handler(
		type_wrapper, message_consumer_link, event_handler_caller_ref );
}

ret_code_t
named_local_mbox_t::subscribe_more_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t * message_consumer_link,
	const event_handler_caller_ref_t & event_handler_caller_ref,
	throwing_strategy_t throwing_strategy )
{
	return m_mbox->subscribe_more_event_handler(
		type_wrapper,
		message_consumer_link,
		event_handler_caller_ref,
		throwing_strategy );
}

ret_code_t
named_local_mbox_t::unsubscribe_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t *
		message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref,
	bool & is_last_subscription,
	throwing_strategy_t throwing_strategy )
{
	return m_mbox->unsubscribe_event_handler(
		type_wrapper,
		message_consumer_link,
		event_handler_caller_ref,
		is_last_subscription,
		throwing_strategy );
}

void
named_local_mbox_t::unsubscribe_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t *
		message_consumer_link )
{
	return m_mbox->unsubscribe_event_handler(
		type_wrapper,
		message_consumer_link );
}

void
named_local_mbox_t::deliver_message(
	const type_wrapper_t & type_wrapper,
	const message_ref_t & message_ref )
{
	m_mbox->deliver_message(
		type_wrapper,
		message_ref );
}

const std::string &
named_local_mbox_t::query_name() const
{
	return m_name;
}

const mbox_t *
named_local_mbox_t::cmp_ordinal() const
{
	return m_mbox->cmp_ordinal();
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
