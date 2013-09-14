/*
	SObjectizer 5.
*/

#include <so_5/rt/impl/h/message_consumer_link.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// message_consumer_link_t
//

message_consumer_link_t::message_consumer_link_t(
	const agent_ref_t & agent_ref )
	:
		m_event_handler_caller( new event_caller_block_t ),
		m_agent_ref( agent_ref )
{
}

message_consumer_link_t::~message_consumer_link_t()
{
}

void
message_consumer_link_t::set_left(
	const message_consumer_link_ref_t & left )
{
	m_left = left;
}

void
message_consumer_link_t::set_right(
	const message_consumer_link_ref_t & right )
{
	m_right = right;
}

void
message_consumer_link_t::dispatch(
	const message_ref_t & message )
{
	agent_t::call_push_event(
		*m_agent_ref,
		m_event_handler_caller,
		message );
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

