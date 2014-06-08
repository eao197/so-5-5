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

namespace
{

template< class C >
auto
try_find_handler(
	C & container,
	const state_t & state ) -> decltype( std::begin(container) )
	{
		typedef decltype(*std::begin(container)) value_t;

		return std::find_if(
				std::begin( container ),
				std::end( container ),
				[&state]( value_t & o ) { return o.m_state == &state; } );
	}

} /* namespace anonymous */

//
// event_caller_block_t
//

event_caller_block_t::event_caller_block_t()
{
}

event_caller_block_t::~event_caller_block_t()
{
}

bool
event_caller_block_t::call(
	const state_t & current_state,
	message_ref_t & message ) const
{
	auto it = try_find_handler( m_states_and_handlers, current_state );

	if( it != std::end( m_states_and_handlers ) )
	{
		it->m_method( message );
		return true;
	}

	return false;
}

void
event_caller_block_t::insert(
	const state_t & target_state,
	event_handler_method_t && method )
{
	auto it = try_find_handler( m_states_and_handlers, target_state );

	if( it != std::end( m_states_and_handlers ) )
		SO_5_THROW_EXCEPTION(
				rc_evt_handler_already_provided,
				"event handler for state is already provided" );

	m_states_and_handlers.emplace_back( target_state, std::move(method) );
}

bool
event_caller_block_t::empty() const
{
	return m_states_and_handlers.empty();
}

void
event_caller_block_t::remove_caller_for_state(
	const state_t & target_state )
{
	m_states_and_handlers.erase(
			try_find_handler( m_states_and_handlers, target_state ) );
}

} /* namespace rt */

} /* namespace so_5 */

