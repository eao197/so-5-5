/*
	SObjectizer 5.
*/

#include <algorithm>

#include <so_5/rt/impl/h/disp_core.hpp>

#include <so_5/disp/one_thread/h/pub.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// disp_core_t
//

disp_core_t::disp_core_t(
	so_environment_t & so_environment,
	const named_dispatcher_map_t & named_dispatcher_map,
	event_exception_logger_unique_ptr_t logger )
	:
		m_so_environment( so_environment ),
		m_default_dispatcher( so_5::disp::one_thread::create_disp() ),
		m_named_dispatcher_map( named_dispatcher_map ),
		m_event_exception_logger( std::move( logger ) )
{
}

disp_core_t::~disp_core_t()
{
}

dispatcher_t &
disp_core_t::query_default_dispatcher()
{
	return *m_default_dispatcher;
}

dispatcher_ref_t
disp_core_t::query_named_dispatcher(
	const std::string & disp_name )
{
	named_dispatcher_map_t::iterator it =
		m_named_dispatcher_map.find( disp_name );

	if( m_named_dispatcher_map.end() != it )
	{
		return it->second;
	}

	return dispatcher_ref_t();
}

void
disp_core_t::start()
{
	m_default_dispatcher->start();

	named_dispatcher_map_t::iterator it = m_named_dispatcher_map.begin();
	named_dispatcher_map_t::iterator it_end = m_named_dispatcher_map.end();

	for( ; it != it_end; ++it )
	{
		it->second->start();
	}
}

void
disp_core_t::finish()
{
	send_shutdown_signal();
	wait_for_full_shutdown();
}

void
disp_core_t::install_exception_logger(
	event_exception_logger_unique_ptr_t logger )
{
	if( nullptr != logger.get() )
	{
		std::lock_guard< std::mutex > lock( m_exception_logger_lock );

		event_exception_logger_unique_ptr_t old_logger;
		old_logger.swap( m_event_exception_logger );
		m_event_exception_logger.swap( logger );

		m_event_exception_logger->on_install( std::move( logger ) );
	}
}

void
disp_core_t::call_exception_logger(
	const std::exception & event_exception,
	const std::string & coop_name )
{
	std::lock_guard< std::mutex > lock( m_exception_logger_lock );

	m_event_exception_logger->log_exception( event_exception, coop_name );
}

void
disp_core_t::send_shutdown_signal()
{
	named_dispatcher_map_t::iterator it = m_named_dispatcher_map.begin();
	named_dispatcher_map_t::iterator it_end = m_named_dispatcher_map.end();

	for( ; it != it_end; ++it )
	{
		it->second->shutdown();
	}

	m_default_dispatcher->shutdown();
}

void
disp_core_t::wait_for_full_shutdown()
{
	named_dispatcher_map_t::iterator it = m_named_dispatcher_map.begin();
	named_dispatcher_map_t::iterator it_end = m_named_dispatcher_map.end();

	for( ; it != it_end; ++it )
	{
		it->second->wait();
	}

	m_default_dispatcher->wait();
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
