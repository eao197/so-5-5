/*
	SObjectizer 5.
*/

#include <so_5/rt/h/state.hpp>

namespace so_5
{

namespace rt
{

//
// state_base_t
//

state_base_t::state_base_t()
{
}

state_base_t::~state_base_t()
{
}

//
// state_t
//

state_t::state_t(
	const agent_t * agent )
	:
		m_target_agent( agent )
{
}

state_t::state_t(
	const agent_t * agent,
	const std::string & state_name )
	:
		m_target_agent( agent ),
		m_state_name( state_name )
{
}

state_t::~state_t()
{
}

bool
state_t::operator == ( const state_t & state ) const
{
	return &state == this;
}

const std::string &
state_t::query_name() const
{
	return m_state_name;
}

bool
state_t::is_target( const agent_t * agent ) const
{
	return m_target_agent == agent;
}

} /* namespace rt */

} /* namespace so_5 */
