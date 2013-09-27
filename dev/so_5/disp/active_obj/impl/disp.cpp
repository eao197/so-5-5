/*
	SObjectizer 5.
*/

#include <cstdlib>
#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/disp/active_obj/impl/h/disp.hpp>
#include <so_5/disp/one_thread/impl/h/disp.hpp>

namespace so_5
{

namespace disp
{

namespace active_obj
{

namespace impl
{

dispatcher_t::dispatcher_t()
{
}

dispatcher_t::~dispatcher_t()
{
}

void
dispatcher_t::start()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
	m_shutdown_started = false;
}

template< class AGENT_DISP >
void
call_shutdown( AGENT_DISP & agent_disp )
{
	agent_disp.second->shutdown();
}

void
dispatcher_t::shutdown()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	// During the shutdown new threads will not be created.
	m_shutdown_started = true;

	std::for_each(
		m_agent_disp.begin(),
		m_agent_disp.end(),
		call_shutdown< agent_disp_map_t::value_type > );
}

template< class AGENT_DISP >
void
call_wait( AGENT_DISP & agent_disp )
{
	agent_disp.second->wait();
}

void
dispatcher_t::wait()
{
	std::for_each(
		m_agent_disp.begin(),
		m_agent_disp.end(),
		call_wait< agent_disp_map_t::value_type > );
}

void
dispatcher_t::put_event_execution_request(
	const so_5::rt::agent_ref_t & agent_ref,
	unsigned int event_count )
{
	// This method shall not be called!
	std::abort();
}

so_5::rt::dispatcher_t &
dispatcher_t::create_disp_for_agent( const so_5::rt::agent_t & agent )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	if( m_shutdown_started )
		throw so_5::exception_t(
			"shutdown was initiated",
			rc_disp_create_failed );

	if( m_agent_disp.end() != m_agent_disp.find( &agent ) )
		throw so_5::exception_t(
			"create dispatcher failed",
			rc_disp_create_failed );

	so_5::rt::dispatcher_ref_t disp(
		new so_5::disp::one_thread::impl::dispatcher_t );

	disp->set_disp_event_exception_handler( query_disp_evt_except_handler() );
	disp->start();
	m_agent_disp[ &agent ] = disp;

	return *disp;
}

void
dispatcher_t::destroy_disp_for_agent( const so_5::rt::agent_t & agent )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	if( !m_shutdown_started )
	{
		agent_disp_map_t::iterator it = m_agent_disp.find( &agent );

		if( m_agent_disp.end() != it )
		{
			it->second->shutdown();
			it->second->wait();
			m_agent_disp.erase( it );
		}
	}
}

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */
