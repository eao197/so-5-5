/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Tools for calling the agent's event handler.
*/


#if !defined( _SO_5__RT__EVENT_HANDLER_CALLER_HPP_ )
#define _SO_5__RT__EVENT_HANDLER_CALLER_HPP_

#include <cstdio>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/event_handler_caller_ref_fwd.hpp>

namespace so_5
{

namespace rt
{

//
// event_handler_caller_t
//

//! The base class for the agent's event handler caller.
class SO_5_TYPE event_handler_caller_t
	:
		private atomic_refcounted_t
{
		friend class smart_atomic_reference_t< event_handler_caller_t >;

		event_handler_caller_t(
			const event_handler_caller_t & );

		void
		operator = (
			const event_handler_caller_t & );

	public:
		event_handler_caller_t();
		virtual ~event_handler_caller_t();

		//! Call agent's event caller.
		virtual void
		call(
			//! Message for the event.
			message_ref_t & message_ref ) const = 0;
};

//
// real_event_handler_caller_t
//

//! Template based implementation of the event_handler_caller.
/*!
 * Allows calling an event handler for messages without actual data
 * (e.g. for signals).
 */
template< class MESSAGE, class AGENT >
class real_event_handler_caller_t
	:
		public event_handler_caller_t
{
	public:
		//! Typedef for the agent method pointer.
		typedef void (AGENT::*FN_PTR_T)(
			const event_data_t< MESSAGE > & );

		real_event_handler_caller_t(
			FN_PTR_T pfn,
			AGENT & agent )
			:
				m_pfn( pfn ),
				m_agent( agent )
		{
		}

		virtual ~real_event_handler_caller_t()
		{}

		virtual void
		call(
			message_ref_t & message_ref ) const
		{
			const event_data_t< MESSAGE > event_data(
				dynamic_cast< MESSAGE * >( message_ref.get() ) );

			(m_agent.*m_pfn)( event_data );
		};

	private:
		//! Agent's method for the handling event.
		FN_PTR_T m_pfn;

		//! Agent for which event should be called.
		AGENT & m_agent;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
