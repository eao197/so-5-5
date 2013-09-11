/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Tools for calling agent's event handler.
*/


#if !defined( _SO_5__RT__EVENT_HANDLER_CALLER_HPP_ )
#define _SO_5__RT__EVENT_HANDLER_CALLER_HPP_

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/message_ref.hpp>

namespace so_5
{

namespace rt
{

class agent_t;
class message_t;


class event_handler_caller_t;

//
// event_handler_caller_t
//

//! Base class for agent's event handler caller.
class SO_5_TYPE event_handler_caller_t
	:
		private atomic_refcounted_t
{
		friend class event_handler_caller_ref_t;

		event_handler_caller_t(
			const event_handler_caller_t & );

		void
		operator = (
			const event_handler_caller_t & );

	public:
		event_handler_caller_t();
		virtual ~event_handler_caller_t();

		//! Call agent's event caller.
		/*!
		 * Event will not be called if agent is in state evere
		 * event is disabled.
		 *
		 * \retval true If event has been called.
		 * \retval false If event has not been called.
		 */
		virtual bool
		call(
			//! Message for the event.
			message_ref_t & message_ref ) const = 0;

		//! Message type identifier.
		virtual type_wrapper_t
		type_wrapper() const = 0;

		//! Value for event caller comparision.
		virtual char *
		ordinal() const = 0;

		//! Value for event caller comparision.
		virtual size_t
		ordinal_size() const = 0;

		//! Target state for the event.
		virtual const state_t *
		target_state() const;
};

//
// real_event_handler_caller_t
//

//! Template based implementation of event_handler_caller.
/*!
 * Allows calling event handler for messages without actual data
 * (e.g. for signals).
 */
template< class MESSAGE, class AGENT >
class real_event_handler_caller_t
	:
		public event_handler_caller_t
{
	public:
		//! Typedef for agent method pointer.
		typedef void (AGENT::*FN_PTR_T)(
			const event_data_t< MESSAGE > & );

		real_event_handler_caller_t(
			FN_PTR_T pfn,
			AGENT & agent,
			const state_t * target_state )
			:
				m_pfn( pfn ),
				m_agent( agent ),
				m_target_state( target_state )
		{}

		virtual ~real_event_handler_caller_t()
		{}

		virtual type_wrapper_t
		type_wrapper() const
		{
			return type_wrapper_t( typeid( MESSAGE ) );
		}

		virtual char *
		ordinal() const
		{
			return (char *) &m_pfn;
		}

		virtual size_t
		ordinal_size() const
		{
			return sizeof( FN_PTR_T );
		}

		virtual const state_t *
		target_state() const
		{
			return m_target_state;
		}

		virtual bool
		call(
			message_ref_t & message_ref ) const
		{
			const bool execute = &m_agent.so_current_state() == m_target_state;

			if( execute )
			{
				const event_data_t< MESSAGE > event_data(
					reinterpret_cast< const MESSAGE * >( message_ref.get() ) );

				(m_agent.*m_pfn)( event_data );
			}

			return execute;
		};

	private:
		//! State for which event is enabled.
		const state_t * const m_target_state;

		//! Agent's method for handling event.
		FN_PTR_T m_pfn;

		//! Agent for which event should be called.
		AGENT & m_agent;
};

//
// not_null_data_real_event_handler_caller_t
//

//! Template based implementation of event_handler_caller.
/*!
 * Doesn't allow calling event handler for messages without actual data
 * (e.g. for signals).
 */
template< class MESSAGE, class AGENT >
class not_null_data_real_event_handler_caller_t
	:
		public event_handler_caller_t
{
	public:
		//! Typedef for agent method pointer.
		typedef void (AGENT::*FN_PTR_T)(
			const not_null_event_data_t< MESSAGE > & );

		not_null_data_real_event_handler_caller_t(
			FN_PTR_T pfn,
			AGENT & agent,
			const state_t * target_state )
			:
				m_pfn( pfn ),
				m_agent( agent ),
				m_target_state( target_state )
		{}

		virtual ~not_null_data_real_event_handler_caller_t()
		{}


		virtual type_wrapper_t
		type_wrapper() const
		{
			return type_wrapper_t( typeid( MESSAGE ) );
		}

		virtual char *
		ordinal() const
		{
			return (char *) &m_pfn;
		}

		virtual size_t
		ordinal_size() const
		{
			return sizeof( FN_PTR_T );
		}

		virtual const state_t *
		target_state() const
		{
			return m_target_state;
		}

		virtual bool
		call(
			message_ref_t & message_ref ) const
		{
			bool execute = &m_agent.so_current_state() == m_target_state;

			if( execute )
			{
				if( message_ref.get() )
				{
					const not_null_event_data_t< MESSAGE > not_null_event_data(
						reinterpret_cast< const MESSAGE & >( *message_ref ) );

					(m_agent.*m_pfn)( not_null_event_data );
				}
			}

			return execute;
		};

	private:
		//! State for which event is enabled.
		const state_t * const m_target_state;

		//! Agent's method for handling event.
		FN_PTR_T m_pfn;

		//! Agent for which event should be called.
		AGENT & m_agent;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
