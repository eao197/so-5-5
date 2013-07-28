/*
	SObjectizer 5.
*/

#include <so_5/rt/h/disp.hpp>

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
	return *m_disp_evt_except_handler;
}

} /* namespace rt */

} /* namespace so_5 */
