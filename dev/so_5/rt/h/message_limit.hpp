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
// one_limit_describer_t
//
template< class M >
struct one_limit_describer_t
	{
		const abort_app_indicator_t< M > abort_app;

		one_limit_describer_t( unsigned int limit )
			:	abort_app( limit )
			{}
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

} /* namespace impl */

//
// redirect_indicator_t
//
/*!
 * \since v.5.5.4
 * \brief Indication that a message must be redirected on overlimit.
 *
 * \tparam MSG Message type of message/signal to be redirected.
 * \tparam LAMBDA Type of lambda- or functional object which returns
 * actual mbox for redirection.
 */
template< typename MSG, typename LAMBDA >
struct redirect_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		//! A lambda/functional object which returns mbox for redirection.
		LAMBDA m_destination_getter;

		redirect_indicator_t(
			unsigned int limit,
			LAMBDA destination_getter )
			:	m_limit( limit )
			,	m_destination_getter( std::move( destination_getter ) )
			{}
	};

template< typename MSG, typename LAMBDA >
void
accept_one_indicator(
	description_container_t & to,
	const redirect_indicator_t< MSG, LAMBDA > & indicator )
	{
		LAMBDA dest_getter = indicator.m_destination_getter;
		to.emplace_back( typeid( MSG ),
				indicator.m_limit,
				[dest_getter]( const overlimit_context_t & ctx ) {
					impl::redirect_reaction( ctx, dest_getter() );
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

//
// message_limit_methods_mixin_t
//
/*!
 * \since v.5.5.4
 * \brief A mixin with message limit definition methods.
 */
struct message_limit_methods_mixin_t
	{
		/*!
		 * \since v.5.5.4
		 * \brief A helper function for creating drop_indicator.
		 */
		template< typename MSG >
		static drop_indicator_t< MSG >
		limit_then_drop( unsigned int limit )
			{
				return drop_indicator_t< MSG >( limit );
			}

		/*!
		 * \since v.5.5.4
		 * \brief A helper function for creating redirect_indicator.
		 */
		template< typename MSG, typename LAMBDA >
		static redirect_indicator_t< MSG, LAMBDA >
		limit_then_redirect(
			unsigned int limit,
			LAMBDA dest_getter )
			{
				return redirect_indicator_t< MSG, LAMBDA >(
						limit,
						std::move( dest_getter ) );
			}

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
		static transform_indicator_t< ARG >
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
		static transform_indicator_t< SOURCE >
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

		/*!
		 * \since v.5.5.4
		 * \brief Helper method for creating message transformation result.
		 */
		template< typename MSG, typename... ARGS >
		static transformed_message_t< MSG >
		make_transformed( mbox_t mbox, ARGS &&... args )
			{
				return transformed_message_t< MSG >::make(
						std::move( mbox ),
						std::forward<ARGS>( args )... );
			}
	};

namespace impl
{

// Forward declaration.
class info_storage_t;

} /* namespace impl */

} /* namespace message_limit */

} /* namespace rt */

} /* namespace so_5 */

