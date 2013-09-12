/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/impl/h/local_mbox.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// local_mbox_t
//

local_mbox_t::local_mbox_t(
	impl::mbox_core_t & mbox_core )
	:
		m_mbox_core( &mbox_core ),
		m_lock( m_mbox_core->allocate_mutex() )
{
}

local_mbox_t::local_mbox_t(
	impl::mbox_core_t & mbox_core,
	ACE_RW_Thread_Mutex & lock )
	:
		m_mbox_core( &mbox_core ),
		m_lock( lock )
{
}

local_mbox_t::~local_mbox_t()
{
	m_mbox_core->deallocate_mutex( m_lock );
}

void
local_mbox_t::subscribe_first_event_handler(
	const type_wrapper_t & type_wrapper,
	std::unique_ptr< impl::message_consumer_link_t > &
		message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );
	m_message_distributor.push_first(
		type_wrapper,
		message_consumer_link,
		event_handler_caller_ref );
}

ret_code_t
local_mbox_t::subscribe_more_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t * message_consumer_link,
	const event_handler_caller_ref_t & event_handler_caller_ref,
	throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	return m_message_distributor.push_more(
		type_wrapper,
		message_consumer_link,
		event_handler_caller_ref,
		throwing_strategy );
}

ret_code_t
local_mbox_t::unsubscribe_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t *
		message_consumer_link,
	const event_handler_caller_ref_t &
		event_handler_caller_ref,
	bool & is_last_subscription,
	throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );
	return m_message_distributor.pop(
		type_wrapper,
		message_consumer_link,
		event_handler_caller_ref,
		is_last_subscription,
		throwing_strategy );
}

void
local_mbox_t::unsubscribe_event_handler(
	const type_wrapper_t & type_wrapper,
	impl::message_consumer_link_t *
		message_consumer_link )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );
	return m_message_distributor.pop(
		type_wrapper,
		message_consumer_link );
}

void
local_mbox_t::deliver_message(
	const type_wrapper_t & type_wrapper,
	const message_ref_t & message_ref )
{
	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );
	m_message_distributor.distribute_message(
		type_wrapper,
		message_ref );
}

const std::string g_mbox_empty_name;

const std::string &
local_mbox_t::query_name() const
{
	return g_mbox_empty_name;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
