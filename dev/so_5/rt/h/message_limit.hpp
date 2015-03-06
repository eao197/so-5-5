/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.4
 * \brief Public part of message limit implementation.
 */

#pragma once

#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/mbox.hpp>

#include <so_5/details/h/lambda_traits.hpp>

#include <so_5/h/declspec.hpp>

#include <functional>
#include <typeindex>
#include <atomic>
#include <vector>

namespace so_5
{

namespace rt
{

namespace message_limit
{

//
// drop_indicator_t
//
template< class M >
struct drop_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		drop_indicator_t( unsigned int limit )
			:	m_limit( limit )
			{}
	};

//
// abort_app_indicator_t
//
template< class M >
struct abort_app_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		abort_app_indicator_t( unsigned int limit )
			:	m_limit( limit )
			{}
	};

//
// redirect_to_mbox_indicator_t
//
/*!
 * \since v.5.5.4
 * \brief Indication that a message must be redirected on overlimit.
 */
template< class M >
struct redirect_to_mbox_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		//! Redirection mbox.
		mbox_t m_destination;

		redirect_to_mbox_indicator_t(
			unsigned int limit,
			mbox_t destination )
			:	m_limit( limit )
			,	m_destination( std::move( destination ) )
			{}
	};

//
// redirect_to_agent_indicator_t
//
/*!
 * \since v.5.5.4
 * \brief Indication that a message must be redirected on overlimit.
 *
 * Redirection is done to the direct mbox of the agent specified.
 */
template< class M >
struct redirect_to_agent_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		//! Redirection agent.
		const agent_t * const m_receiver;

		redirect_to_agent_indicator_t(
			unsigned int limit,
			const agent_t * receiver )
			:	m_limit( limit )
			,	m_receiver( receiver )
			{}
	};

//
// one_limit_describer_t
//
template< class M >
struct one_limit_describer_t
	{
		const drop_indicator_t< M > drop;
		const abort_app_indicator_t< M > abort_app;

		one_limit_describer_t( unsigned int limit )
			:	drop( limit )
			,	abort_app( limit )
			{}

		//! Message must be redirected to another destination
		//! on overflow.
		redirect_to_mbox_indicator_t< M >
		redirect( so_5::rt::mbox_t mbox )
			{
				if( !mbox )
					SO_5_THROW_EXCEPTION( rc_empty_mbox_for_redirection,
							std::string( "unable to redirect message to "
								"empty mbox, msg_type: " ) +
							typeid( M ).name() );

				return redirect_to_mbox_indicator_t< M >{ drop.m_limit, mbox };
			}

		//! Message must be redirected to an agent on overflow.
		redirect_to_agent_indicator_t< M >
		redirect( const agent_t * to )
			{
				return redirect_to_agent_indicator_t< M >{ drop.m_limit, to };
			}
	};

//
// description_t
//
struct description_t
	{
		//! Type of message.
		std::type_index m_msg_type;

		//! Max count of waiting messages.
		unsigned int m_limit;	

		//! Reaction to overload.
		action_t m_action;

		//! Initializing constructor.
		description_t(
			std::type_index msg_type,
			unsigned int limit,
			action_t action )
			:	m_msg_type( std::move( msg_type ) )
			,	m_limit( limit )
			,	m_action( std::move( action ) )
			{}
	};

//
// description_container_t
//
using description_container_t = std::vector< description_t >;

//
// accept_one_indicator
//
template< class M >
void
accept_one_indicator(
	description_container_t & to,
	const drop_indicator_t< M > & indicator )
	{
		to.emplace_back( typeid( M ),
				indicator.m_limit,
				[]( const overlimit_context_t & ) {} );
	}

namespace impl
{

/*!
 * \since v.5.5.4
 * \brief Actual implementation of abort application reaction.
 */
SO_5_FUNC
void
abort_app_reaction( const overlimit_context_t & ctx );

} /* namespace impl */

template< class M >
void
accept_one_indicator(
	description_container_t & to,
	const abort_app_indicator_t< M > & indicator )
	{
		to.emplace_back( typeid( M ),
				indicator.m_limit,
				[]( const overlimit_context_t & ctx ) {
					impl::abort_app_reaction( ctx );
				} );
	}

namespace impl
{

/*!
 * \since v.5.5.4
 * \brief Actual implementation of redirect message reaction.
 */
SO_5_FUNC
void
redirect_reaction(
	//! Context on which overlimit must be handled.
	const overlimit_context_t & ctx,
	//! Destination for message redirection.
	const mbox_t & to );

/*!
 * \since v.5.5.4
 * \brief Actual implementation of redirect message reaction.
 */
SO_5_FUNC
void
redirect_reaction(
	//! Context on which overlimit must be handled.
	const overlimit_context_t & ctx,
	//! A receiver for the redirected message.
	const agent_t & to );

} /* namespace impl */

template< class M >
void
accept_one_indicator(
	description_container_t & to,
	const redirect_to_mbox_indicator_t< M > & indicator )
	{
		const mbox_t destination = indicator.m_destination;
		to.emplace_back( typeid( M ),
				indicator.m_limit,
				[destination]( const overlimit_context_t & ctx ) {
					impl::redirect_reaction( ctx, destination );
				} );
	}

template< class M >
void
accept_one_indicator(
	description_container_t & to,
	const redirect_to_agent_indicator_t< M > & indicator )
	{
		const agent_t * const receiver = indicator.m_receiver;
		to.emplace_back( typeid( M ),
				indicator.m_limit,
				[receiver]( const overlimit_context_t & ctx ) {
					impl::redirect_reaction( ctx, *receiver );
				} );
	}

namespace impl
{

/*!
 * \since v.5.5.4
 * \brief Actual implementation of transform reaction.
 */
SO_5_FUNC
void
transform_reaction(
	//! Context on which overlimit must be handled.
	const overlimit_context_t & ctx,
	//! Destination for new message.
	const mbox_t & to,
	//! Type of new message.
	const std::type_index & msg_type,
	//! An instance of new message.
	const message_ref_t & message );

template< bool is_signal, typename RESULT, typename MSG, typename... ARGS >
struct transformed_message_maker
	{
		static RESULT
		make( mbox_t mbox, ARGS &&... args )
			{
				return RESULT( std::move( mbox ),
						so_5::rt::details::make_message_instance< MSG >(
								std::forward<ARGS>( args )... ) );
			}
	};

template< typename RESULT, typename MSG >
struct transformed_message_maker< true, RESULT, MSG >
	{
		static RESULT
		make( mbox_t mbox )
			{
				return RESULT( std::move( mbox ) );
			}
	};

} /* namespace impl */

//
// transformed_message_t
//
/*!
 * \since v.5.5.4
 * \brief A result of message transformation.
 *
 * \tparam MSG Type of result (transformed) message.
 */
template< typename MSG >
class transformed_message_t
	{
	public :
		//! Initializing constructor for the case when MSG is a message type.
		transformed_message_t(
			//! Message box to which transformed message to be sent.
			mbox_t mbox,
			//! New message instance.
			std::unique_ptr< MSG > msg )
			:	m_mbox( std::move( mbox ) )
			{
				ensure_message_with_actual_data( msg.get() );

				m_message = message_ref_t( msg.release() );
			}
		//! Initializing constructor for the case when MSG is a signal type.
		transformed_message_t(
			//! Message box to which signal to be sent.
			mbox_t mbox )
			:	m_mbox( std::move( mbox ) )
			{}

		//! Destination message box.
		const mbox_t &
		mbox() const { return m_mbox; }

		//! Type of the transformed message.
		std::type_index
		msg_type() const { return typeid(MSG); }

		//! Instance of transformed message.
		/*!
		 * \note Will be nullptr for signal.
		 */
		const message_ref_t &
		message() const { return m_message; }

		//! A helper method for transformed_message construction.
		template< typename... ARGS >
		static transformed_message_t< MSG >
		make( mbox_t mbox, ARGS &&... args )
			{
				return impl::transformed_message_maker<
							is_signal< MSG >::value,
							transformed_message_t,
							MSG,
							ARGS...
						>::make(
								std::move( mbox ),
								std::forward<ARGS>( args )... );
			}

	private :
		//! Destination message box.
		mbox_t m_mbox;

		//! Instance of transformed message.
		/*!
		 * \note Will be nullptr for signal.
		 */
		message_ref_t m_message;
	};

//
// transform_indicator_t
//
/*!
 * \since v.5.5.4
 * \brief An indicator of transform reaction on message overlimit.
 *
 * \tparam SOURCE Type of message to be transformed.
 */
template< typename SOURCE >
struct transform_indicator_t
	{
		//! Limit value.
		unsigned int m_limit;

		//! Reaction.
		action_t m_action;

		//! Initializing constructor.
		transform_indicator_t(
			unsigned int limit,
			action_t action )
			:	m_limit( limit )
			,	m_action( std::move( action ) )
			{}
	};

//
// limit_then_transform
//
/*!
 * \since v.5.5.4
 * \brief A helper function for creating transform_indicator.
 *
 * Must be used for message transformation. Type of message is
 * detected automatically from the type of transformation lambda
 * argument.
 */
template<
		typename LAMBDA,
		typename ARG = typename so_5::details::lambda_traits::
				argument_type_if_lambda< LAMBDA >::type >
transform_indicator_t< ARG >
limit_then_transform(
	unsigned int limit,
	LAMBDA transformator )
	{
		ensure_not_signal< ARG >();

		action_t act = [transformator]( const overlimit_context_t & ctx ) {
				const ARG & msg = dynamic_cast< const ARG & >(
						*ctx.m_message.get() );
				auto r = transformator( msg );
				impl::transform_reaction(
						ctx, r.mbox(), r.msg_type(), r.message() );
			};

		return transform_indicator_t< ARG >{ limit, std::move( act ) };
	}

/*!
 * \since v.5.5.4
 * \brief A helper function for creating transform_indicator.
 *
 * Must be used for signal transformation. Type of signal must be
 * explicitely specified.
 */
template< typename SOURCE, typename LAMBDA >
transform_indicator_t< SOURCE >
limit_then_transform(
	unsigned int limit,
	LAMBDA transformator )
	{
		ensure_signal< SOURCE >();

		action_t act = [transformator]( const overlimit_context_t & ctx ) {
				auto r = transformator();
				impl::transform_reaction(
						ctx, r.mbox(), r.msg_type(), r.message() );
			};

		return transform_indicator_t< SOURCE >{ limit, std::move( act ) };
	}

template< class M >
void
accept_one_indicator(
	description_container_t & to,
	const transform_indicator_t< M > & indicator )
	{
		to.emplace_back( typeid( M ), indicator.m_limit, indicator.m_action );
	}

//
// accept_indicators
//
template< typename I, typename... ARGS >
void
accept_indicators(
	description_container_t & to,
	I && indicator,
	ARGS &&... others )
	{
		accept_one_indicator( to, std::forward< I >( indicator ) );

		accept_indicators( to, std::forward< ARGS >( others )... );
	}

inline void
accept_indicators(
	description_container_t & )
	{
	}

namespace impl
{

// Forward declaration.
class info_storage_t;

} /* namespace impl */

} /* namespace message_limit */

} /* namespace rt */

} /* namespace so_5 */

