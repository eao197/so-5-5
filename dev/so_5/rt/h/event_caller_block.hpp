/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Wrapper around a bunch of the event callers definition.
*/

#if !defined( _SO_5__RT__EVENT_CALLER_BLOCK_HPP_ )
#define _SO_5__RT__EVENT_CALLER_BLOCK_HPP_

#include <map>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/agent_ref_fwd.hpp>
#include <so_5/rt/h/event_handler_caller_ref_fwd.hpp>

namespace so_5
{

namespace rt
{

//
// event_caller_block_t
//

//! Wrapper around a bunch of event callers.
class SO_5_TYPE event_caller_block_t
	:
		private atomic_refcounted_t
{
		friend class smart_atomic_reference_t< event_caller_block_t >;

	public:
		event_caller_block_t();
		~event_caller_block_t();

		//! Call handler for message if defined for that state.
		void
		call(
			const state_t & current_state,
			message_ref_t & message_ref ) const;

		//! Add caller.
		/*!
		 * \throw exception_t if state is already has a caller.
		 */
		void
		insert(
			//! State in which event should be called.
			const state_t & target_state,
			//! Event to be called.
			const event_handler_caller_ref_t & ehc_ref );

	private:
		//! Type of map from state to event_caller.
		typedef std::map<
					const state_t *,
					event_handler_caller_ref_t >
			map_t;

		//! Event handlers.
		map_t m_callers;
};

//
// event_caller_block_ref_t
//
/*!
 * \brief A smart reference to event_caller_block.
 * \note Defined as typedef since v.5.2.0.
 */
typedef smart_atomic_reference_t< event_caller_block_t >
		event_caller_block_ref_t;

//
// demand_handler_pfn_t
//
/*!
 * \since v.5.2.0
 * \brief Demand handler prototype.
 */
typedef void (*demand_handler_pfn_t)(
		message_ref_t &,
		const event_caller_block_ref_t &,
		agent_t * );

} /* namespace rt */

} /* namespace so_5 */

#endif

