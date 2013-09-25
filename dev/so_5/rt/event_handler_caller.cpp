/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/message.hpp>

namespace so_5
{

namespace rt
{

//
// event_handler_caller_t
//

event_handler_caller_t::event_handler_caller_t()
{
}

event_handler_caller_t::~event_handler_caller_t()
{
}

const state_t *
event_handler_caller_t::target_state() const
{
	return 0;
}

} /* namespace rt */

} /* namespace so_5 */
