/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of the template for the subcription to messages.
*/

#if !defined( _SO_5__RT__SUBSCRIPTION_BIND_HPP_ )
#define _SO_5__RT__SUBSCRIPTION_BIND_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>
#include <so_5/h/types.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>

namespace so_5
{

namespace rt
{

//
// agent_owns_state()
//

//! Checks the agent owns the state specified.
SO_5_EXPORT_FUNC_SPEC( void )
ensure_agent_owns_state(
	//! Agent to be checked.
	agent_t & agent,
	//! State to check.
	const state_t * state );

//! Is agent convertible to specified type.
/*!
 * \tparam AGENT Target type of type conversion to be checked.
 */
template< class AGENT >
AGENT &
try_cast_agent_to(
	//! Object to be checked.
	agent_t & agent )
{
	AGENT * casted_agent = dynamic_cast< AGENT * >( &agent );

	// Was conversion successful?
	if( nullptr == casted_agent )
	{
		// No. Actual type of the agent is not convertible to the AGENT.
		SO_5_THROW_EXCEPTION(
			rc_agent_incompatible_type_conversion,
			std::string( "Unable convert agent to type: " ) +
				typeid(AGENT).name() );
	}

	return *casted_agent;
}

//
// subscription_bind_t
//

/*!
 * \brief A class for creating a subscription to messages from the mbox.
*/
class SO_5_TYPE subscription_bind_t
{
	public:
		subscription_bind_t(
			//! Agent to subscribe.
			agent_t & agent,
			//! Mbox for messages to be subscribed.
			const mbox_ref_t & mbox_ref );

		~subscription_bind_t();

		//! Set up a state in which events are allowed be processed.
		subscription_bind_t &
		in(
			//! State in which events are allowed.
			const state_t & state );

		//! Make subscription to the message.
		template< class MESSAGE, class AGENT >
		void
		event(
			//! Event handling method.
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) )
		{
			// Agent must be owner of the state.
			ensure_agent_owns_state( m_agent, m_state );

			// Agent must have right type.
			AGENT & casted_agent = try_cast_agent_to< AGENT >( m_agent );

			event_handler_caller_ref_t event_handler_caller_ref(
				new real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					casted_agent,
					m_state ) );

			create_event_subscription(
				type_wrapper_t( typeid( MESSAGE ) ),
				m_mbox_ref,
				event_handler_caller_ref );
		}

	private:
		//! Create an event subscription.
		void
		create_event_subscription(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Mbox for messages.
			mbox_ref_t & mbox_ref,
			//! Event caller.
			const event_handler_caller_ref_t & ehc );

		//! Agent to which we are subscribing.
		agent_t & m_agent;
		//! Mbox for messages to subscribe.
		mbox_ref_t m_mbox_ref;
		//! State for events.
		const state_t * m_state;
};

} /* namespace rt */

} /* namespace so_5 */

#endif

