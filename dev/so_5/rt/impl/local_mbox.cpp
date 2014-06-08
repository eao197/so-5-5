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
local_mbox_t::subscribe_event_handler(
	const std::type_index & type_wrapper,
	agent_t * subscriber,
	const event_caller_block_ref_t & event_caller )
{
	// Since v.5.2.3.4 there is no locking inside the method!

	m_subscribers[ type_wrapper ][ subscriber ] = event_caller;
}

void
local_mbox_t::unsubscribe_event_handlers(
	const std::type_index & type_wrapper,
	agent_t * subscriber )
{
	// Since v.5.2.3.4 there is no locking inside the method!

	auto it = m_subscribers.find( type_wrapper );
	if( it != m_subscribers.end() )
	{
		it->second.erase( subscriber );
		if( it->second.empty() )
			m_subscribers.erase( it );
	}
}

void
local_mbox_t::deliver_message(
	const std::type_index & type_wrapper,
	const message_ref_t & message_ref ) const
{
	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	auto it = m_subscribers.find( type_wrapper );
	if( it != m_subscribers.end() )
	{
		for( auto s = it->second.begin(), e = it->second.end(); s != e; ++s )
			agent_t::call_push_event( *(s->first), s->second, message_ref );
	}
}

void
local_mbox_t::read_write_lock_acquire()
{
	if( -1 == m_lock.acquire_write() )
		SO_5_THROW_EXCEPTION( rc_unexpected_error,
				"ACE_RW_Thread_Mutex::acquire_write() failed." );
}

void
local_mbox_t::read_write_lock_release()
{
	if( -1 == m_lock.release() )
		SO_5_THROW_EXCEPTION( rc_unexpected_error,
				"ACE_RW_Thread_Mutex::release() failed." );
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

			auto it = m_subscribers.find( type_index );
			if( it != m_subscribers.end() )
			{
				auto f = it->second.begin();
				if( f == it->second.end() )
					SO_5_THROW_EXCEPTION(
							so_5::rc_no_svc_handlers,
							"no service handlers [ACTUAL SUBSCRIBERS MAP EMPTY!]" );

				auto s = ++(it->second.begin());
				if( s != it->second.end() )
					SO_5_THROW_EXCEPTION(
							so_5::rc_more_than_one_svc_handler,
							"more than one service handler found" );

				agent_t::call_push_service_request(
						*(f->first),
						f->second,
						svc_request_ref );
			}
			else
				SO_5_THROW_EXCEPTION(
						so_5::rc_no_svc_handlers,
						"no service handlers (no subscribers for message)" );
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

