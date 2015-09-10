/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.1
 * \brief Implementation of free functions send/send_delayed.
 */

#pragma once

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/environment.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

	/*
	 * This is helpers for so_5::send implementation.
	 */

	template< class MESSAGE, bool IS_SIGNAL >
	struct instantiator_and_sender_t
		{
		};

	template< class MESSAGE >
	struct instantiator_and_sender_t< MESSAGE, true >
		{
			void
			send( const so_5::rt::mbox_t & to )
				{
					to->deliver_signal< MESSAGE >();
				}

			void
			send_delayed(
				so_5::rt::environment_t & env,
				const so_5::rt::mbox_t & to,
				std::chrono::steady_clock::duration pause )
				{
					env.single_timer< MESSAGE >( to, pause );
				}

			timer_id_t
			send_periodic(
				so_5::rt::environment_t & env,
				const so_5::rt::mbox_t & to,
				std::chrono::steady_clock::duration pause,
				std::chrono::steady_clock::duration period )
				{
					return env.schedule_timer< MESSAGE >( to, pause, period );
				}
		};

} /* namespace impl */

} /* namespace rt */

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a message.
 */
template< typename MESSAGE, typename... ARGS >
void
send( const so_5::rt::mbox_t & to, ARGS&&... args )
	{
		to->deliver_message(
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward<ARGS>(args)...) );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for sending a signal.
 */
template< typename MESSAGE >
void
send( const so_5::rt::mbox_t & to )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		helper.send( to );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a message to
 * the agent's direct mbox.
 */
template< typename MESSAGE, typename... ARGS >
void
send_to_agent( const so_5::rt::agent_t & receiver, ARGS&&... args )
	{
		send< MESSAGE >( receiver.so_direct_mbox(), std::forward<ARGS>(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a message to
 * the ad-hoc agent's direct mbox.
 */
template< typename MESSAGE, typename... ARGS >
void
send_to_agent(
	const so_5::rt::adhoc_agent_definition_proxy_t & receiver,
	ARGS&&... args )
	{
		send< MESSAGE >( receiver.direct_mbox(), std::forward<ARGS>(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for sending a signal to the agent's direct mbox.
 */
template< typename MESSAGE >
void
send_to_agent( const so_5::rt::agent_t & receiver )
	{
		send< MESSAGE >( receiver.so_direct_mbox() );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for sending a signal to the ad-hoc agent's direct mbox.
 */
template< typename MESSAGE >
void
send_to_agent( const so_5::rt::adhoc_agent_definition_proxy_t & receiver )
	{
		send< MESSAGE >( receiver.direct_mbox() );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		env.single_timer(
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward<ARGS>(args)... ),
				to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >( agent.so_environment(), to, pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a delayed message
 * to the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal.
 */
template< typename MESSAGE >
void
send_delayed(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		helper.send_delayed( env, to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >( agent.so_environment(), to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal to
 * the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for delivering a delayed signal to
 * the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return env.schedule_timer( 
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward< ARGS >( args )... ),
				to, pause, period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				to,
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a periodic message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal.
 */
template< typename MESSAGE >
timer_id_t
send_periodic(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		return helper.send_periodic( env, to, pause, period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				to,
				pause,
				period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal to
 * the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				period );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for delivering a periodic signal to
 * the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				period );
	}

/*!
 * \name Helper functions for simplification of synchronous interactions.
 * \{
 */

namespace make_async_details {

inline const so_5::rt::mbox_t &
arg_to_mbox( const so_5::rt::mbox_t & mbox ) { return mbox; }

inline const so_5::rt::mbox_t &
arg_to_mbox( const so_5::rt::agent_t & agent ) { return agent.so_direct_mbox(); }

inline const so_5::rt::mbox_t &
arg_to_mbox( so_5::rt::adhoc_agent_definition_proxy_t & agent ) { return agent.direct_mbox(); }

} /* namespace details */

/*!
 * \since v.5.5.9
 * \brief Make a synchronous request and receive result in form of a future
 * object.
 */
template< typename RESULT, typename MSG, typename MBOX, typename... ARGS >
std::future< RESULT >
make_async_get(
	//! Mbox for sending a synchronous request to.
	MBOX && mbox,
	//! Arguments for MSG's constructor params.
	ARGS &&... args )
	{
		using namespace make_async_details;

		so_5::rt::ensure_not_signal< MSG >();

		return arg_to_mbox( std::forward< MBOX >(mbox) )
				->template get_one< RESULT >()
				.template make_async< MSG >( std::forward< ARGS >(args)... );
	}

template<
		typename RESULT,
		typename SIGNAL,
		typename MBOX,
		typename RV = typename std::enable_if<
				so_5::rt::is_signal< SIGNAL >::value,
				std::future< RESULT > >::type >
RV
make_async_get(
	//! Mbox for sending a synchronous request to.
	MBOX && mbox )
	{
		using namespace make_async_details;

		so_5::rt::ensure_signal< SIGNAL >();

		return arg_to_mbox( std::forward< MBOX >(mbox) )
				->template get_one< RESULT >()
				.template async< SIGNAL >();
	}
/*!
 * \}
 */

} /* namespace so_5 */

