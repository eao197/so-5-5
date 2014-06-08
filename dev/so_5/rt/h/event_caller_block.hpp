/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Wrapper around a bunch of the event callers definition.
*/

#if !defined( _SO_5__RT__EVENT_CALLER_BLOCK_HPP_ )
#define _SO_5__RT__EVENT_CALLER_BLOCK_HPP_

#include <vector>
#include <functional>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/agent_ref_fwd.hpp>

namespace so_5
{

namespace rt
{

//
// event_handler_method_t
//
/*!
 * \since v.5.3.0
 * \brief Type of event handler method.
 */
typedef std::function< void(message_ref_t &) > event_handler_method_t;

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
		/*!
		 * \retval true event handler has been found and called.
		 * \retval false event handler has not been found (i.e. it is
		 * disabled in \a current_state).
		 */
		bool
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
			event_handler_method_t && method );

		/*!
		 * \since v.5.2.3
		 * \brief Is there no any callers?
		 */
		bool
		empty() const;

		/*!
		 * \since v.5.2.3
		 * \brief Remove caller for the state specified.
		 */
		void
		remove_caller_for_state(
			const state_t & target_state );

	private:
		/*!
		 * \since v.5.3.0
		 * \brief Pair of state and handler for that state.
		 */
		struct state_and_handler_t
			{
				const state_t * m_state;
				event_handler_method_t m_method;

				state_and_handler_t(
					const state_t & state,
					event_handler_method_t && method )
					:	m_state( &state )
					,	m_method( std::move(method) )
					{}

				state_and_handler_t(
					state_and_handler_t && o )
					:	m_state( o.m_state )
					,	m_method( std::move(o.m_method) )
					{}
			};

		/*!
		 * \since v.5.3.0
		 * \brief Type of container for state_and_handler objects.
		 */
		typedef std::vector< state_and_handler_t > state_and_handler_container_t;

		/*!
		 * \since v.5.3.0
		 * \brief List of states and handlers for them.
		 */
		state_and_handler_container_t m_states_and_handlers;

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
		const event_caller_block_t *,
		agent_t * );

} /* namespace rt */

} /* namespace so_5 */

#endif

