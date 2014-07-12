/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/agent.hpp>
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
	impl::mbox_core_t & mbox_core,
	mbox_id_t id )
	:	m_mbox_core( &mbox_core )
	,	m_id( id )
	,	m_lock( m_mbox_core->allocate_mutex() )
{
}

local_mbox_t::local_mbox_t(
	impl::mbox_core_t & mbox_core,
	mbox_id_t id,
	ACE_RW_Thread_Mutex & lock )
	:	m_mbox_core( &mbox_core )
	,	m_id( id )
	,	m_lock( lock )
{
}

local_mbox_t::~local_mbox_t()
{
	m_mbox_core->deallocate_mutex( m_lock );
}

void
local_mbox_t::subscribe_event_handler(
	const std::type_index & type_wrapper,
	agent_t * subscriber )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	m_subscribers.emplace( type_wrapper, subscriber );
}

void
local_mbox_t::unsubscribe_event_handlers(
	const std::type_index & type_wrapper,
	agent_t * subscriber )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	m_subscribers.erase(
			subscribers_set_t::value_type( type_wrapper, subscriber ) );
}

void
local_mbox_t::deliver_message(
	const std::type_index & type_wrapper,
	const message_ref_t & message_ref ) const
{
	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	auto it = m_subscribers.lower_bound(
			subscribers_set_t::value_type( type_wrapper, 0 ) );

	while( it != m_subscribers.end() && it->first == type_wrapper )
	{
		agent_t::call_push_event(
				*(it->second), m_id, type_wrapper, message_ref );
		++it;
	}
}

void
local_mbox_t::deliver_service_request(
	const std::type_index & type_index,
	const message_ref_t & svc_request_ref ) const
{
	msg_service_request_base_t & svc_request =
			*(dynamic_cast< msg_service_request_base_t * >(
					svc_request_ref.get() ));

	try
		{
			ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

			auto it = m_subscribers.lower_bound(
					subscribers_set_t::value_type( type_index, 0 ) );
			
			if( it == m_subscribers.end() )
				SO_5_THROW_EXCEPTION(
						so_5::rc_no_svc_handlers,
						"no service handlers (no subscribers for message)" );

			auto next = it;
			++next;
			if( next != m_subscribers.end() && next->first == type_index )
				SO_5_THROW_EXCEPTION(
						so_5::rc_more_than_one_svc_handler,
						"more than one service handler found" );

				agent_t::call_push_service_request(
						*(it->second), m_id, type_index, svc_request_ref );
		}
	catch( ... )
		{
			svc_request.set_exception( std::current_exception() );
		}
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

