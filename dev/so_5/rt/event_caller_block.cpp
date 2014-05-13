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

event_caller_block_t::~event_caller_block_t()
{
}

void
event_caller_block_t::call(
	const state_t & current_state,
	message_ref_t & message ) const
{
	auto it = m_callers.find( &current_state );
	if( it != m_callers.end() )
		it->second->call( message );
}

void
event_caller_block_t::insert(
	const state_t & target_state,
	const event_handler_caller_ref_t & ehc_ref )
{
	auto it = m_callers.find( &target_state );
	if( it != m_callers.end() )
		SO_5_THROW_EXCEPTION(
				rc_evt_handler_already_provided,
				"event handler for state is already provided" );

	m_callers[ &target_state ] = ehc_ref;
}

bool
event_caller_block_t::empty() const
{
	return m_callers.empty();
}

void
event_caller_block_t::remove_caller_for_state(
	const state_t & target_state )
{
	m_callers.erase( &target_state );
}

} /* namespace rt */

} /* namespace so_5 */

