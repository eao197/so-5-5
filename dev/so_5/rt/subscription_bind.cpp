/*
	SObjectizer 5.
*/

#include <so_5/rt/h/subscription_bind.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace rt
{

//
// agent_owns_state()
//

SO_5_EXPORT_FUNC_SPEC(void)
ensure_agent_owns_state(
	agent_t & agent,
	const state_t * state )
{
	if( !state->is_target( &agent ) )
	{
		SO_5_THROW_EXCEPTION(
			rc_agent_is_not_the_state_owner,
			"agent doesn't own the state" );
	}
}

//
// subscription_bind_t
//

subscription_bind_t::subscription_bind_t(
	agent_t & agent,
	const mbox_ref_t & mbox_ref )
	:
		m_agent( agent ),
		m_mbox_ref( mbox_ref ),
		m_state( &m_agent.so_default_state() )
{
}

subscription_bind_t::~subscription_bind_t()
{
}

subscription_bind_t &
subscription_bind_t::in(
	const state_t & state )
{
	m_state = &state;
	return *this;
}

void
subscription_bind_t::create_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc )
{
	return m_agent.create_event_subscription(
		type_wrapper,
		mbox_ref,
		ehc );
}

} /* namespace rt */

} /* namespace so_5 */

