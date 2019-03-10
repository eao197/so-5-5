/*
 * SObjectizer-5
 */

/*!
 * \since
 * v.5.5.9
 *
 * \file
 * \brief Helper class for accessing private functionality of environment-class.
 */

#pragma once

#include <so_5/rt/h/environment.hpp>

namespace so_5 {

namespace impl {

//
// internal_env_iface_t
//
/*
 * NOTE: the implementation of this class is in environment.cpp file.
 */
/*!
 * \since
 * v.5.5.9
 *
 * \brief A helper class for accessing the functionality of
 * environment-class which is specific for SObjectizer internals only.
 */
class SO_5_TYPE internal_env_iface_t
	{
		//! Environment instance to work with.
		environment_t & m_env;

	public :
		//! Initializing constructor.
		internal_env_iface_t( environment_t & env )
			:	m_env( env )
			{}

		//! Create multi-producer/single-consumer mbox.
		mbox_t
		create_mpsc_mbox(
			//! The only consumer for the messages.
			agent_t * single_consumer,
			//! Pointer to the optional message limits storage.
			//! If this pointer is null then the limitless MPSC-mbox will be
			//! created. If this pointer is not null the the MPSC-mbox with limit
			//! control will be created.
			const so_5::message_limit::impl::info_storage_t * limits_storage );

		//! Notification about readiness to the deregistration.
		void
		ready_to_deregister_notify(
			//! Cooperation which is ready to be deregistered.
			coop_t * coop );

		//! Do the final actions of a cooperation deregistration.
		void
		final_deregister_coop(
			//! Cooperation name to be deregistered.
			const std::string & coop_name );

		//! Is message delivery tracing enabled?
		bool
		is_msg_tracing_enabled() const;

		//! Get access to message delivery tracer stuff holder.
		/*!
		 * \throw exception_t if message delivery tracing is disabled.
		 */
		so_5::msg_tracing::holder_t &
		msg_tracing_stuff() const;

		//! Get default lock_factory for MPSC queues.
		/*!
		 * \since
		 * v.5.5.18
		 */
		so_5::disp::mpsc_queue_traits::lock_factory_t
		default_mpsc_queue_lock_factory() const;

		//! Get default lock_factory for MPMC queues.
		/*!
		 * \since
		 * v.5.5.18
		 */
		so_5::disp::mpmc_queue_traits::lock_factory_t
		default_mpmc_queue_lock_factory() const;

		/*!
		 * \name Methods for working with event_queue_hooks
		 * \{
		 */
		/*!
		 * \brief Call the event_queue_hook when an agent is being bound
		 * to a particular event_queue.
		 *
		 * An agent should call this method when it is being bound to a
		 * particular event queue.
		 *
		 * \attention
		 * The pointer returned should be used as pointer to event queue.
		 *
		 * \since
		 * v.5.5.24
		 */
		SO_5_NODISCARD
		event_queue_t *
		event_queue_on_bind(
			agent_t * agent,
			event_queue_t * original_queue ) SO_5_NOEXCEPT;

		/*!
		 * \brief Call the event_queue_hook when an agent is being unbound
		 * from its event_queue.
		 *
		 * An agent should call this method when it is being unbound from its
		 * event queue.
		 *
		 * \attention
		 * The value \a queue should be exactly the value returned from
		 * previous call to event_queue_on_bind().
		 *
		 * \since
		 * v.5.5.24
		 */
		void
		event_queue_on_unbind(
			agent_t * agent,
			event_queue_t * queue ) SO_5_NOEXCEPT;
		/*!
		 * \}
		 */
	};

/*!
 * \brief Helper function to be used for extraction of lock_factory for MPSC queues.
 *
 * \note Intended to be used in template code:
 * \code
template< typename Disp_Params >
auto
safe_get_lock_factory( so_5::environment_t & env, Disp_Params & params )
{
	auto lf = params.queue_params().lock_factory();
	if( !lf )
		lf = so_5::impl::default_lock_factory( env, lf );
	return lf;
}
 * \endcode
 * A call to default_lock_factory() will return appropriate lock_factory
 * for MPSC or MPMC queues.
 *
 * \since
 * v.5.5.18
 */
inline so_5::disp::mpsc_queue_traits::lock_factory_t
default_lock_factory(
	environment_t & env,
	const so_5::disp::mpsc_queue_traits::lock_factory_t & )
	{
		return internal_env_iface_t{ env }.default_mpsc_queue_lock_factory();
	}

/*!
 * \brief Helper function to be used for extraction of lock_factory for MPSC queues.
 *
 * \note Intended to be used in template code:
 * \code
template< typename Disp_Params >
auto
safe_get_lock_factory( so_5::environment_t & env, Disp_Params & params )
{
	auto lf = params.queue_params().lock_factory();
	if( !lf )
		lf = so_5::impl::default_lock_factory( env, lf );
	return lf;
}
 * \endcode
 * A call to default_lock_factory() will return appropriate lock_factory
 * for MPSC or MPMC queues.
 *
 * \since
 * v.5.5.18
 */
inline so_5::disp::mpmc_queue_traits::lock_factory_t
default_lock_factory(
	environment_t & env,
	const so_5::disp::mpmc_queue_traits::lock_factory_t & )
	{
		return internal_env_iface_t{ env }.default_mpmc_queue_lock_factory();
	}

//
// wrap_init_fn_call
//
/*!
 * \brief A special wrapper for calling init function.
 *
 * This wrapper calls init function and catch exception.
 * If an exception is derived from std::exception it will be rethrown.
 * If an exception is caught by catch(...) statement then an instance
 * of so_5::exception_t will be thrown instead of caught exception.
 *
 * \since
 * v.5.5.24.3
 */
template< typename Init_Fn >
void
wrap_init_fn_call( Init_Fn init_fn )
	{
		try
			{
				init_fn();
			}
		catch( const std::exception & )
			{
				throw; // Exception derived from std::exception
					// will be caught automatically.
			}
		catch( ... )
			{
				SO_5_THROW_EXCEPTION(
						rc_unknown_exception_type,
						"exception of unknown type is thrown from init function" );
			}
	}

} /* namespace impl */

} /* namespace so_5 */

