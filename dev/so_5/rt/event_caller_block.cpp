/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

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
	evt_caller_array_t evt_caller_array;
	evt_caller_array_t::iterator
		it = m_callers_array.begin(),
		it_end = m_callers_array.end();

	for(; it != it_end; ++it )
	{
		if( !( *it == ehc_ref ) )
			evt_caller_array.push_back( *it );
	}

	m_callers_array.swap( evt_caller_array );
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
	evt_caller_array_t::const_iterator
		it = m_callers_array.begin(),
		it_end = m_callers_array.end();

	for(; it != it_end; ++it )
	{
		if( *it == ehc_ref )
			return true;
	}

	return false;
}

const event_handler_caller_t *
event_caller_block_t::find(
	const event_handler_caller_ref_t & ehc_ref ) const
{
	evt_caller_array_t::const_iterator
		it = m_callers_array.begin(),
		it_end = m_callers_array.end();

	for(; it != it_end; ++it )
	{
		if( *it == ehc_ref )
			return it->get();
	}

	return nullptr;
}

bool
event_caller_block_t::is_empty() const
{
	return m_callers_array.empty();
}

} /* namespace rt */

} /* namespace so_5 */
