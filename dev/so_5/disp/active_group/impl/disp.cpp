/*
	SObjectizer 5.
*/

#include <cstdlib>
#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/disp/active_group/impl/h/disp.hpp>
#include <so_5/disp/one_thread/impl/h/disp.hpp>

namespace so_5
{

namespace disp
{

namespace active_group
{

namespace impl
{

dispatcher_t::dispatcher_t()
{
}

dispatcher_t::~dispatcher_t()
{
}

ret_code_t
dispatcher_t::start()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
	m_shutdown_started = false;
	return 0;
}

template< class AGENT_DISP >
void
call_shutdown( AGENT_DISP & agent_disp )
{
	agent_disp.second.m_disp_ref->shutdown();
}

void
dispatcher_t::shutdown()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	// Начинаем завершение работы,
	// с этого момента, новые диспетчеры под группы агентов не создаются,
	// а старые не уничтожаются.
	m_shutdown_started = true;

	std::for_each(
		m_group_disp.begin(),
		m_group_disp.end(),
		call_shutdown< active_group_disp_map_t::value_type > );
}

template< class AGENT_DISP >
void
call_wait( AGENT_DISP & agent_disp )
{
	agent_disp.second.m_disp_ref->wait();
}

void
dispatcher_t::wait()
{
	std::for_each(
		m_group_disp.begin(),
		m_group_disp.end(),
		call_wait< active_group_disp_map_t::value_type > );
}

void
dispatcher_t::put_event_execution_request(
	const so_5::rt::agent_ref_t &,
	unsigned int )
{
	// Этот метод не должен вызываться.
	std::abort();
}

so_5::rt::dispatcher_t &
dispatcher_t::query_disp_for_group( const std::string & group_name )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	if( m_shutdown_started )
		throw so_5::exception_t(
			"shutdown was initiated",
			rc_disp_create_failed );

	active_group_disp_map_t::iterator it =
		m_group_disp.find( group_name );

	// Если уже есть диспетчер для активной группы, то вернем его.
	if( m_group_disp.end() != it )
	{
		++(it->second.m_user_agent);
		return *(it->second.m_disp_ref);
	}

	// Если надо создавать агента, то создаем его.
	so_5::rt::dispatcher_ref_t disp(
		new so_5::disp::one_thread::impl::dispatcher_t );

	disp->start();
	m_group_disp[ group_name ] = disp_with_ref_t( disp, 1 );

	return *disp;
}

void
dispatcher_t::release_disp_for_group( const std::string & group_name )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	// Если не начат shutdown, то помечаем, что
	// в активной группе стало на 1 агент меньше.
	if( !m_shutdown_started )
	{
		active_group_disp_map_t::iterator it = m_group_disp.find( group_name );

		if( m_group_disp.end() != it &&
			0 == --(it->second.m_user_agent) )
		{
			it->second.m_disp_ref->shutdown();
			it->second.m_disp_ref->wait();
			m_group_disp.erase( it );
		}
	}
}

} /* namespace impl */

} /* namespace active_group */

} /* namespace disp */

} /* namespace so_5 */
