/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

#include <algorithm>

namespace so_5
{

namespace rt
{

//
// event_caller_block_t
//

event_caller_block_t::event_caller_block_t()
{
}

event_caller_block_t::event_caller_block_t(
	const event_caller_block_t & event_caller_block )
	:
		m_callers_array( event_caller_block.m_callers_array )
{
}

event_caller_block_t::~event_caller_block_t()
{
}

void
event_caller_block_t::erase(
	const event_handler_caller_ref_t & ehc_ref )
{
	m_callers_array.erase(
			std::remove(
					m_callers_array.begin(),
					m_callers_array.end(),
					ehc_ref ),
			m_callers_array.end() );
}

void
event_caller_block_t::insert(
	const event_handler_caller_ref_t & ehc_ref )
{
	m_callers_array.push_back( ehc_ref );
}

bool
event_caller_block_t::has(
	const event_handler_caller_ref_t & ehc_ref ) const
{
	return m_callers_array.end() != std::find(
			m_callers_array.begin(),
			m_callers_array.end(),
			ehc_ref );
}

const event_handler_caller_t *
event_caller_block_t::find(
	const event_handler_caller_ref_t & ehc_ref ) const
{
	auto it = std::find(
			m_callers_array.begin(),
			m_callers_array.end(),
			ehc_ref );
	return m_callers_array.end() != it ?  it->get() : nullptr;
}

bool
event_caller_block_t::is_empty() const
{
	return m_callers_array.empty();
}

} /* namespace rt */

} /* namespace so_5 */

