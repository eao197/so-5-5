/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Various stuff for creation of event handlers.
 */

#pragma once

#include <so_5/h/exception.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/details/h/lambda_traits.hpp>

#include <so_5/rt/h/execution_demand.hpp>

namespace so_5 {

namespace rt {

namespace details {

/*!
 * \since v.5.3.0
 * \brief Various helpers for event subscription.
 */
namespace event_subscription_helpers
{

/*!
 * \brief Get actual agent pointer.
 *
 * \throw exception_t if dynamic_cast fails.
 */
template< class AGENT >
AGENT *
get_actual_agent_pointer( agent_t & agent )
{
	// Agent must have right type.
	AGENT * cast_result = dynamic_cast< AGENT * >( &agent );

	// Was conversion successful?
	if( nullptr == cast_result )
	{
		// No. Actual type of the agent is not convertible to the AGENT.
		SO_5_THROW_EXCEPTION(
			rc_agent_incompatible_type_conversion,
			std::string( "Unable convert agent to type: " ) +
				typeid(AGENT).name() );
	}

	return cast_result;
}

/*!
 * \brief Get actual msg_service_request pointer.
 *
 * \throw exception_t if dynamic_cast fails.
 */
template< class RESULT, class MESSAGE >
msg_service_request_t<
		RESULT,
		typename message_payload_type< MESSAGE >::envelope_type > *
get_actual_service_request_pointer(
	const message_ref_t & message_ref )
{
	using actual_request_msg_t =
			msg_service_request_t<
					RESULT,
					typename message_payload_type< MESSAGE >::envelope_type >;

	auto actual_request_ptr = dynamic_cast< actual_request_msg_t * >(
			message_ref.get() );

	if( !actual_request_ptr )
		SO_5_THROW_EXCEPTION(
				rc_msg_service_request_bad_cast,
				std::string( "unable cast msg_service_request "
						"instance to appropriate type, "
						"expected type is: " ) +
				typeid(actual_request_msg_t).name() );

	return actual_request_ptr;
}

} /* namespace event_subscription_helpers */

/*!
 * \since v.5.3.0
 * \brief Internal namespace for details of agent method invocation implementation.
 */
namespace promise_result_setting_details
{

using namespace so_5::details::lambda_traits;

template< class RESULT >
struct result_setter_t
	{
		template< class AGENT, class PARAM >
		void
		call_old_format_event_and_set_result(
			std::promise< RESULT > & to,
			AGENT * a,
			RESULT (AGENT::*pfn)( const event_data_t< PARAM > & ),
			const event_data_t< PARAM > & evt )
			{
				to.set_value( (a->*pfn)( evt ) );
			}

		template< class AGENT, class METHOD, class PARAM >
		void
		call_new_format_event_and_set_result(
			std::promise< RESULT > & to,
			AGENT * a,
			METHOD pfn,
			const PARAM & msg )
			{
				to.set_value( (a->*pfn)( msg ) );
			}

		template< class AGENT >
		void
		call_new_format_signal_and_set_result(
			std::promise< RESULT > & to,
			AGENT * a,
			RESULT (AGENT::*pfn)() )
			{
				to.set_value( (a->*pfn)() );
			}

		template< class LAMBDA, class PARAM >
		void
		call_event_lambda_and_set_result(
			std::promise< RESULT > & to,
			LAMBDA l,
			const PARAM & msg )
			{
				to.set_value( traits< LAMBDA >::call_with_arg( l, msg ) );
			}

		template< class LAMBDA >
		void
		call_signal_lambda_and_set_result(
			std::promise< RESULT > & to,
			LAMBDA l )
			{
				to.set_value( traits< LAMBDA >::call_without_arg( l ) );
			}
	};

template<>
struct result_setter_t< void >
	{
		template< class AGENT, class PARAM >
		void
		call_old_format_event_and_set_result(
			std::promise< void > & to,
			AGENT * a,
			void (AGENT::*pfn)( const event_data_t< PARAM > & ),
			const event_data_t< PARAM > & evt )
			{
				(a->*pfn)( evt );
				to.set_value();
			}

		template< class AGENT, class METHOD, class PARAM >
		void
		call_new_format_event_and_set_result(
			std::promise< void > & to,
			AGENT * a,
			METHOD pfn,
			const PARAM & msg )
			{
				(a->*pfn)( msg );
				to.set_value();
			}

		template< class AGENT >
		void
		call_new_format_signal_and_set_result(
			std::promise< void > & to,
			AGENT * a,
			void (AGENT::*pfn)() )
			{
				(a->*pfn)();
				to.set_value();
			}

		template< class LAMBDA, class PARAM >
		void
		call_event_lambda_and_set_result(
			std::promise< void > & to,
			LAMBDA l,
			const PARAM & msg )
			{
				traits< LAMBDA >::call_with_arg( l, msg );
				to.set_value();
			}

		template< class LAMBDA >
		void
		call_signal_lambda_and_set_result(
			std::promise< void > & to,
			LAMBDA l )
			{
				traits< LAMBDA >::call_without_arg( l );
				to.set_value();
			}
	};

} /* namespace promise_result_setting_details */

} /* namespace details */

} /* namespace rt */

//
// handler
//
//FIXME: examples of usage of this function must be provided in Doxygen comment.
/*!
 * \since v.5.5.13
 * \brief A function for creation event handler.
 *
 * \note Must be used for the case when message is an ordinary message.
 */
template< class LAMBDA >
rt::details::msg_type_and_handler_pair_t
handler( LAMBDA && lambda )
	{
		using namespace so_5::rt;
		using namespace so_5::rt::details;
		using namespace so_5::details::lambda_traits;
		using namespace so_5::rt::details::event_subscription_helpers;
		using namespace so_5::rt::details::promise_result_setting_details;

		typedef traits< typename std::decay< LAMBDA >::type > TRAITS;
		typedef typename TRAITS::result_type RESULT;
		typedef typename TRAITS::argument_type MESSAGE;

		ensure_not_signal< MESSAGE >();

		auto method = [lambda](
				invocation_type_t invocation_type,
				message_ref_t & message_ref)
			{
				if( invocation_type_t::service_request == invocation_type )
					{
						auto actual_request_ptr =
								get_actual_service_request_pointer< RESULT, MESSAGE >(
										message_ref );

						auto msg = message_payload_type< MESSAGE >::extract_payload_ptr(
								actual_request_ptr->m_param );
						ensure_message_with_actual_data( msg );

						// All exceptions will be processed in service_handler_on_message.
						result_setter_t< RESULT >().call_event_lambda_and_set_result(
								actual_request_ptr->m_promise,
								lambda,
								*msg );
					}
				else
					{
						auto msg = message_payload_type< MESSAGE >::extract_payload_ptr(
								message_ref );
						ensure_message_with_actual_data( msg );

						TRAITS::call_with_arg( lambda, *msg );
					}
			};

		return msg_type_and_handler_pair_t{
				message_payload_type< MESSAGE >::payload_type_index(),
				method };
	}

//
// handler
//
//FIXME: examples of usage of this function must be provided in Doxygen comment.
/*!
 * \since v.5.5.13
 * \brief A function for creation event handler.
 *
 * \note Must be used for the case when message is a signal.
 */
template< class SIGNAL, class LAMBDA >
rt::details::msg_type_and_handler_pair_t
handler( LAMBDA && lambda )
	{
		using namespace so_5::rt;
		using namespace so_5::rt::details;
		using namespace so_5::details::lambda_traits;
		using namespace so_5::rt::details::event_subscription_helpers;
		using namespace so_5::rt::details::promise_result_setting_details;

		ensure_signal< SIGNAL >();

		typedef traits< typename std::decay< LAMBDA >::type > TRAITS;
		typedef typename TRAITS::result_type RESULT;

		auto method = [lambda](
				invocation_type_t invocation_type,
				message_ref_t & message_ref)
			{
				if( invocation_type_t::service_request == invocation_type )
					{
						auto actual_request_ptr =
								get_actual_service_request_pointer< RESULT, SIGNAL >(
										message_ref );

						// All exceptions will be processed in service_handler_on_message.
						result_setter_t< RESULT >().call_signal_lambda_and_set_result(
								actual_request_ptr->m_promise,
								lambda );
					}
				else
					{
						TRAITS::call_without_arg( lambda );
					}
			};

		return msg_type_and_handler_pair_t{
				message_payload_type< SIGNAL >::payload_type_index(),
				method };
	}

} /* namespace so_5 */

