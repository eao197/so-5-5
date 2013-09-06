/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Agent state listener definition.
*/

#if !defined( _SO_5__RT__AGENT_STATE_LISTENER_HPP_ )
#define _SO_5__RT__AGENT_STATE_LISTENER_HPP_

#include <memory>

#include <cpp_util_2/h/nocopy.hpp>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/state.hpp>

namespace so_5
{

namespace rt
{

class state_t;

//
// agent_state_listener_t
//

//! Interface of agent state listener.
/*!
 * This interface intended for cases where agent state switches
 * should be observed or monitored. A "state listener" is an object
 * which could be attached to an agent and the agent will inform
 * listener about state changes. Such state listeners should implement
 * this interface.
 *
 * A non-limited counte of state listeners could be attached to an agent.
 * For each of them agent will call changed() method inside
 * so_5::rt::agent_t::so_change_state().
 *
 * \attention It is important not to change agent state inside changed().
 * Because this could lead to infinite recursion.
 */
class SO_5_TYPE agent_state_listener_t
	:
		private cpp_util_2::nocopy_t
{
	public:
		virtual ~agent_state_listener_t();

		//! Hook method for state changes.
		/*!
		 * Agent calls this method after successful change of state.
		 */
		virtual void
		changed(
			//! Agent which state has been changed.
			agent_t & agent,
			//! New agent state.
			const state_t & state ) = 0;
};

//! Typedef for agent_state_listener autopointer.
typedef std::unique_ptr< agent_state_listener_t >
	agent_state_listener_unique_ptr_t;

//! Typedef for agent_state_listener smart pointer.
typedef std::shared_ptr< agent_state_listener_t >
	agent_state_listener_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

