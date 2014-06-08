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
	const type_wrapper_t & type_wrapper,
	agent_t * subscriber,
	const event_caller_block_ref_t & event_caller )
{
	// Since v.5.2.3.4 there is no locking inside the method!

	m_subscribers[ type_wrapper ][ subscriber ] = event_caller;
}

void
local_mbox_t::unsubscribe_event_handlers(
	const type_wrapper_t & type_wrapper,
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
	const type_wrapper_t & type_wrapper,
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

const std::string g_mbox_empty_name;

const std::string &
local_mbox_t::query_name() const
{
	return g_mbox_empty_name;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

