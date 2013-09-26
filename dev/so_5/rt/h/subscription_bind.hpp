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
#include <so_5/h/throwing_strategy.hpp>
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

//! Does agent own this state?
SO_5_EXPORT_FUNC_SPEC( ret_code_t )
agent_owns_state(
	//! Agent to be checked.
	agent_t & agent,
	//! State to check.
	const state_t * state,
	//! Exception strategy.
	throwing_strategy_t throwing_strategy );

//! Is agent convertible to specified type.
/*!
 * \tparam AGENT Target type of type conversion to be checked.
 */
template< class AGENT >
ret_code_t
agent_convertable_to(
	//! Object to be checked.
	agent_t * agent,
	//! Receiver of the casted pointer (in case if the conversion is available).
	AGENT * & casted_agent,
	//! Exception strategy.
	throwing_strategy_t throwing_strategy )
{
	ret_code_t res = 0;
	casted_agent = dynamic_cast< AGENT * >( agent );

	// Was conversion successful?
	if( nullptr == casted_agent )
	{
		// No actual type of the agent is not convertible to the AGENT.
		res = rc_agent_incompatible_type_conversion;

		// Handling exception strategy.
		if( THROW_ON_ERROR == throwing_strategy )
		{
			std::string error_msg = "Unable convert agent to type ";
			const std::type_info & ti = typeid( AGENT );
			error_msg += ti.name();
			throw exception_t( error_msg, res );
		}
	}

	return res;
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

		//! Make subscription to message.
		template< class MESSAGE, class AGENT >
		ret_code_t
		event(
			//! Event handling method.
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ),
			//! Exception strategy.
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			// Agent must be owner of the state.
			ret_code_t res = agent_owns_state(
				m_agent,
				m_state,
				throwing_strategy );

			if( res )
				// This is possible only if throwing_strategy != THROW_ON_ERROR.
				// So simply return error code.
				return res;

			AGENT * casted_agent = nullptr;
			// Agent must have right type.
			res = agent_convertable_to< AGENT >(
				&m_agent,
				casted_agent,
				throwing_strategy );

			if( res )
				return res;

			event_handler_caller_ref_t event_handler_caller_ref(
				new real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					*casted_agent,
					m_state ) );

			return create_event_subscription(
				type_wrapper_t( typeid( MESSAGE ) ),
				m_mbox_ref,
				event_handler_caller_ref,
				throwing_strategy );
		}

	private:
		//! Create an event subscription.
		ret_code_t
		create_event_subscription(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Mbox for messages.
			mbox_ref_t & mbox_ref,
			//! Event caller.
			const event_handler_caller_ref_t & ehc,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

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

