/*
	SObjectizer 5.
*/

#include <so_5/util/h/apply_throwing_strategy.hpp>

#include <so_5/rt/h/subscription_bind.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace rt
{

//
// agent_owns_state()
//

SO_5_EXPORT_FUNC_SPEC( ret_code_t )
agent_owns_state(
	agent_t & agent,
	const state_t * state,
	throwing_strategy_t throwing_strategy )
{
	if( !state->is_target( &agent ) )
	{
		return so_5::util::apply_throwing_strategy(
			rc_agent_is_not_the_state_owner,
			throwing_strategy,
			"agent doesn't own the state" );
	}

	return 0;
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

ret_code_t
subscription_bind_t::create_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc,
	throwing_strategy_t throwing_strategy )
{
	return m_agent.create_event_subscription(
		type_wrapper,
		mbox_ref,
		ehc,
		throwing_strategy );
}

//
// subscription_unbind_t
//

subscription_unbind_t::subscription_unbind_t(
	agent_t & agent,
	const mbox_ref_t & mbox_ref )
	:
		m_agent( agent ),
		m_mbox_ref( mbox_ref ),
		m_state( &m_agent.so_default_state() )
{
}

subscription_unbind_t::~subscription_unbind_t()
{
}

subscription_unbind_t &
subscription_unbind_t::in(
	const state_t & state )
{
	m_state = &state;
	return *this;
}

ret_code_t
subscription_unbind_t::destroy_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc,
	throwing_strategy_t throwing_strategy )
{
	return m_agent.destroy_event_subscription(
		type_wrapper,
		mbox_ref,
		ehc,
		throwing_strategy );
}

} /* namespace rt */

} /* namespace so_5 */

