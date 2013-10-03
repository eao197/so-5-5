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

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/agent_ref_fwd.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>

namespace so_5
{

namespace rt
{

//
// event_caller_block_t
//

//! Typedef for the event_handler_caller_ref container.
typedef std::vector< event_handler_caller_ref_t > evt_caller_array_t;

//! Wrapper around a bunch of event callers.
class SO_5_TYPE event_caller_block_t
	:
		private atomic_refcounted_t
{
		friend class smart_atomic_reference_t< event_caller_block_t >;

	public:
		event_caller_block_t();
		event_caller_block_t(
			const event_caller_block_t & event_caller_block );

		~event_caller_block_t();

		//! Call every handler for a message.
		void
		call(
			message_ref_t & message_ref ) const;

		//! Erase caller.
		void
		erase( const event_handler_caller_ref_t & ehc_ref );

		//! Add caller.
		void
		insert( const event_handler_caller_ref_t & ehc_ref );

		//! Is this caller already added.
		bool
		has( const event_handler_caller_ref_t & ehc_ref ) const;

		//! Search for the caller.
		const event_handler_caller_t *
		find( const event_handler_caller_ref_t & ehc_ref ) const;

		//! Is this bunch empty?
		bool
		is_empty() const;

	private:
		//! Bunch of callers.
		evt_caller_array_t m_callers_array;
};

inline void
event_caller_block_t::call(
	message_ref_t & message_ref ) const
{
	evt_caller_array_t::const_iterator it = m_callers_array.begin();
	evt_caller_array_t::const_iterator it_end = m_callers_array.end();

	for(; it != it_end; ++it )
		if( (*it)->call( message_ref ) )
			break;
}

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

