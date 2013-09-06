/*
	SObjectizer 5.
*/

#include <so_5/rt/h/disp.hpp>

#include <so_5/h/exception.hpp>

namespace so_5
{

namespace rt
{

//
// disp_evt_except_handler_t
//

disp_evt_except_handler_t::~disp_evt_except_handler_t()
{}

//
// dispatcher_t
//

dispatcher_t::dispatcher_t()
	:	m_disp_evt_except_handler( nullptr )
{
}

dispatcher_t::~dispatcher_t()
{
}

void
dispatcher_t::set_disp_event_exception_handler(
	disp_evt_except_handler_t & disp_evt_except_handler )
{
	m_disp_evt_except_handler = &disp_evt_except_handler;
}

disp_evt_except_handler_t &
dispatcher_t::query_disp_evt_except_handler()
{
	if( !m_disp_evt_except_handler )
		throw so_5::exception_t(
				"call to dispatcher_t::query_disp_evt_except_handler() when "
				"evt_except_handler is null",
				so_5::rc_unexpected_error );

	return *m_disp_evt_except_handler;
}

} /* namespace rt */

} /* namespace so_5 */
