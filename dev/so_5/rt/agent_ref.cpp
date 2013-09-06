/*
	SObjectizer 5.
*/

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace rt
{

agent_ref_t::agent_ref_t()
	:
		m_agent_ptr( nullptr )
{
}

agent_ref_t::agent_ref_t(
	agent_t * agent )
	:
		m_agent_ptr( agent )
{
	inc_agent_ref_count();
}

agent_ref_t::agent_ref_t(
	const agent_ref_t & agent_ref )
	:
		m_agent_ptr( agent_ref.m_agent_ptr )
{
	inc_agent_ref_count();
}

agent_ref_t &
agent_ref_t::operator = ( const agent_ref_t & agent_ref )
{
	if( &agent_ref != this )
	{
		dec_agent_ref_count();

		m_agent_ptr = agent_ref.m_agent_ptr;
		inc_agent_ref_count();
	}

	return *this;
}

agent_ref_t &
agent_ref_t::operator = ( agent_ref_t && o )
{
	if( &o != this )
	{
		dec_agent_ref_count();

		m_agent_ptr = o.release_pointer();
	}

	return *this;
}

agent_ref_t::~agent_ref_t()
{
	dec_agent_ref_count();
}

inline void
agent_ref_t::inc_agent_ref_count()
{
	if( m_agent_ptr )
		m_agent_ptr->inc_ref_count();
}

inline void
agent_ref_t::dec_agent_ref_count()
{
	if( m_agent_ptr &&
		0 == m_agent_ptr->dec_ref_count() )
	{
		delete m_agent_ptr;
		m_agent_ptr = nullptr;
	}
}

} /* namespace rt */

} /* namespace so_5 */
