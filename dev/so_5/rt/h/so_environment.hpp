/*
	SObjectizer 5.
*/

/*!
	\file
	\brief SObjectizer Environment definition.
*/

#if !defined( _SO_5__RT__SO_ENVIRONMENT_HPP_ )
#define _SO_5__RT__SO_ENVIRONMENT_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/disp_binder.hpp>
#include <so_5/rt/h/so_layer.hpp>
#include <so_5/rt/h/coop_listener.hpp>
#include <so_5/rt/h/event_exception_logger.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

#include <so_5/timer_thread/h/timer_thread.hpp>
#include <so_5/timer_thread/h/timer_id.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class so_environment_impl_t;

} /* namespace impl */

class type_wrapper_t;

//
// so_environment_params_t
//

//! Parameters for the SObjectizer Environment initialization.
/*!
 * This class is used for setting SObjectizer Parameters.
 *
 * \see http://www.parashift.com/c++-faq/named-parameter-idiom.html
 */
class SO_5_TYPE so_environment_params_t
{
		friend class impl::so_environment_impl_t;

	public:
		/*!
		 * \brief Constructor.
		 *
		 * Sets default values for parameters.
		 */
		so_environment_params_t();
		~so_environment_params_t();

		//! Set mutex pool size for the syncronizing work with mboxes.
		/*!
		 * A ACE_RW_Thread_Mutex is necessary to work with mboxes.
		 * However there are can be so many mboxes and lifetime
		 * of them can be so small that it is not efficient to
		 * create a dedicated mutex for every mbox.
		 *
		 * A mutex pool is used by the SObjectizer Environment for this
		 * purpose.
		 *
		 * This method allows change the default mutex pool size.
		 *
		 * \see so_environment_t::create_local_mbox().
		 */
		so_environment_params_t &
		mbox_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Set mutex pool size for the syncronizing work with agent cooperations.
		/*!
		 * A work with cooperations requires syncronization.
		 * The SObjectizer Environment uses a pool of mutexes to do this.
		 *
		 * This method allows change the default mutex pool size.
		 */
		so_environment_params_t &
		agent_coop_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Set mutex pool size for the syncronizing work with event queues.
		/*!
		 * A work with the local agent event queues requires syncronization.
		 * SObjectizer Environment uses pool of mutexes to do that.
		 *
		 * This method allows change the default mutex pool size.
		 */
		so_environment_params_t &
		agent_event_queue_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Add a named dispatcher.
		/*!
		 * By default the SObjectizer Environment has only one dispatcher
		 * with one working thread. A user can add his own dispatcher:
		 * named ones.
		 *
		 * \note If a dispatcher with \a name is already registered it
		 * will be replaced by a new dispatcher \a dispatcher.
		 */
		so_environment_params_t &
		add_named_dispatcher(
			//! Dispatcher name.
			const nonempty_name_t & name,
			//! Dispatcher.
			dispatcher_unique_ptr_t && dispatcher );

		//! Set the timer thread.
		/*!
		 * If \a timer_thread is a nullptr then default timer thread
		 * will be used.
		 */
		so_environment_params_t &
		timer_thread(
			//! Timer thread to be set.
			so_5::timer_thread::timer_thread_unique_ptr_t && timer_thread );

		//! Add an additional layer to the SObjectizer Environment.
		/*!
		 * If this layer is already added it will be replaced by \a layer_ptr.
		 * 
		 * The method distinguishes layers from each other by the type SO_LAYER.
		*/
		template< class SO_LAYER >
		so_environment_params_t &
		add_layer(
			//! A layer to be added.
			std::unique_ptr< SO_LAYER > && layer_ptr )
		{
			if( layer_ptr.get() )
			{
				so_layer_unique_ptr_t ptr( layer_ptr.release() );

				add_layer(
					type_wrapper_t( typeid( SO_LAYER ) ),
					std::move( ptr ) );
			}

			return *this;
		}

		so_environment_params_t &
		coop_listener(
			coop_listener_unique_ptr_t && coop_listener );

		so_environment_params_t &
		event_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		so_environment_params_t &
		event_exception_handler(
			event_exception_handler_unique_ptr_t && handler );

		unsigned int
		mbox_mutex_pool_size() const;

		unsigned int
		agent_coop_mutex_pool_size() const;

		unsigned int
		agent_event_queue_mutex_pool_size() const;

		const named_dispatcher_map_t &
		named_dispatcher_map() const;

		const so_layer_map_t &
		so_layers_map() const;

	private:
		//! Add an additional layer.
		/*!
		 * If this layer is already added it will be replaced by \a layer_ptr.
		 * 
		 * The method distinguishes layers from each other by the type SO_LAYER.
		 */
		void
		add_layer(
			//! Type identification for layer.
			const type_wrapper_t & type,
			//! A layer to be added.
			so_layer_unique_ptr_t layer_ptr );

		//! Pool size of mutexes to be used with mboxes.
		unsigned int m_mbox_mutex_pool_size;

		//! Pool size of mutexes to be used with agent cooperations.
		unsigned int m_agent_coop_mutex_pool_size;

		//! Pool size of mutexes to be used with agent local queues.
		unsigned int m_agent_event_queue_mutex_pool_size;

		//! Named dispatchers.
		named_dispatcher_map_t m_named_dispatcher_map;

		//! Timer thread.
		so_5::timer_thread::timer_thread_unique_ptr_t m_timer_thread;

		//! Additional layers.
		so_layer_map_t m_so_layers;

		//! Cooperation listener.
		coop_listener_unique_ptr_t m_coop_listener;

		//! Exception logger.
		event_exception_logger_unique_ptr_t m_event_exception_logger;

		//! Exception handler.
		event_exception_handler_unique_ptr_t m_event_exception_handler;
};

//
// so_environment_t
//

//! SObjectizer Environment.
/*!
 * \section so_env__intro Basic information
 *
 * The SObjectizer Environment provides a basic infrastructure for
 * the SObjectizer Run-Time execution.
 *
 * The main method of starting SObjectizer Environment creates a
 * class derived from the so_environment_t and reimplementing the
 * so_environment_t::init() method.
 * This method should be used to define starting actions of
 * application. For example first application cooperations can
 * be registered here and starting messages can be sent to them.
 *
 * The SObjectizer Environment calls the so_environment_t::init() when
 * the SObjectizer Run-Time is successfully started. 
 * If something happened during Run-Time startup then 
 * the method init() will not be called.
 *
 * The SObjectizer Run-Time is started by the so_environment_t::run().
 * This method blocks the caller thread until SObjectizer completely
 * finished its work.
 *
 * The SObjectizer Run-Time is finished by the so_environment_t::stop().
 * This method doesn't block the caller thread. Instead it sends a special
 * shutdown signal to the Run-Time. The SObjectizer Run-Time then 
 * informs agents about this and waits finish of agents work.
 * The SObjectizer Run-Time finishes if all agents are stopped and
 * all cooperations are deregistered.
 *
 * Methods of the SObjectizer Environment can be splitted into the
 * following groups:
 * - working with mboxes;
 * - working with dispatchers, exception loggers and handlers;
 * - working with cooperations;
 * - working with delayed and periodic messages;
 * - working with additional layers;
 * - initializing/running/stopping/waiting of the Run-Time.
 *
 * \section so_env__mbox_methods Methods for working with mboxes.
 *
 * SObjectizer Environment allows creation of named and anonymous mboxes.
 * Syncronization objects for these mboxes can be obtained from
 * common pools or assigned by a user during mbox creation.
 *
 * Mboxes are created by so_environment_t::create_local_mbox() methods.
 * All these methods return the mbox_ref_t which is a smart reference 
 * to the mbox.
 *
 * An anonymous mbox is automatically destroyed when the last reference to it is
 * destroyed. So, to save the anonymous mbox, the mbox_ref from 
 * the create_local_mbox() should be stored somewhere.
 *
 * Named mbox must be destroyed manually by calling the 
 * so_environment_t::destroy_mbox() method. But physically the deletion of the 
 * named mbox postponed to the deletion of last reference to it. 
 * So if there is some reference to the named mbox it instance will live 
 * with this reference. But mbox itself will be removed from 
 * SObjectizer Environment lists.
 *
 *
 * \section so_env__coop_methods Methods for working with cooperations.
 *
 * Cooperations can be created by so_environment_t::create_coop() methods.
 *
 * The method so_environment_t::register_coop() should be used for the 
 * cooperation registration.
 *
 * Method so_environment_t::deregister_coop() should be used for the 
 * cooperation deregistration.
 *
 * \section so_env__delayed_message_methods Methods for 
 * sending delayed and periodic messages.
 *
 * Receiving of delayed and/or periodic messages are named as timer events.
 * 
 * The timer event can be created and destroyed. If the timer event for
 * a delayed message is destroyed before message timeout is expired then
 * message delivery will be canceled. For periodic messages destroying of
 * the timer event means that message delivery will be stopped.
 *
 * Timer events can be created by so_environment_t::schedule_timer()
 * methods. The one version of the schedule_timer() is intended for
 * messages with an actual data. The second one -- for the signals without
 * the message data.
 *
 * Methods schedule_timer() return a special reference for the timer event.
 * Timer event destroyed when this reference destroyed. So it is necessary
 * to store this reference somewhere. Also the timer event can be destroyed
 * by the so_5::timer_thread::timer_id_ref_t::release() method.
 *
 * A special method so_environment_t::single_timer() can be used in
 * case when a single shot timer event is necessary. With using this
 * method there is no need to store reference for the scheduled
 * single shot timer event.
 */
class SO_5_TYPE so_environment_t
{
		//! Auxiliary methods for getting reference to itself.
		/*!
		 * Could be used in constructors without compiler warnings.
		 */
		so_environment_t &
		self_ref();

	public:
		explicit so_environment_t(
			//! Initialization params.
			const so_environment_params_t & so_environment_params );

		virtual ~so_environment_t();

		/*!
		 * \name Methods for working with mboxes.
		 * \{
		 */

		//! Create an anonymous mbox with the default mutex.
		mbox_ref_t
		create_local_mbox();

		//! Create named mbox with the default mutex.
		/*!
		 * If \a mbox_name is unique then a new mutex will be created.
		 * If not the reference to existing mutex is returned.
		 */
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & mbox_name );

		//! Create an anonymous mbox with the user supplied mutex.
		/*!
		 * Intended for cases when a dedicated mutex for the mbox
		 * is necessary.
		 */
		mbox_ref_t
		create_local_mbox(
			//! A dedicated mutex for the mbox.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );

		//! Create a named mbox with the user supplied mutex.
		/*!
		 * Intended for cases when a dedicated mutex for the mbox
		 * is necessary.
		 */
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & mbox_name,
			//! A dedicated mutex for the mbox.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );
		/*!
		 * \}
		 */

		/*!
		 * \name Method for working with dispatchers.
		 * \{
		 */

		//! Access to the default dispatcher.
		dispatcher_t &
		query_default_dispatcher();

		//! Get named dispatcher.
		/*!
		 * \return A reference to the dispatcher with the name \a disp_name.
		 * Zero reference if dispatcher with such name is not found.
		 */
		dispatcher_ref_t
		query_named_dispatcher(
			//! Name of dispatcher.
			const std::string & disp_name );

		//! Set up an exception logger.
		void
		install_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		//! Set up an exception handler.
		void
		install_exception_handler(
			event_exception_handler_unique_ptr_t && handler );
		/*!
		 * \}
		 */

		/*!
		 * \name Methods for working with cooperations.
		 * \{
		 */

		//! Create a cooperation.
		/*!
		 * \return A new cooperation with \a name. This cooperation
		 * will use default dispatcher binders.
		 */
		agent_coop_unique_ptr_t
		create_coop(
			//! A new cooperation name.
			const nonempty_name_t & name );

		//! Create a cooperation.
		/*!
		 * A binder \a disp_binder will be used for binding cooperation
		 * agents to the dispatcher. This binder will be default binder for
		 * this cooperation.
		 *
			\code
			so_5::rt::agent_coop_unique_ptr_t coop = so_env.create_coop(
				so_5::rt::nonempty_name_t( "some_coop" ),
				so_5::disp::active_group::create_disp_binder(
					"active_group",
					"some_active_group" ) );

			// That agent will be bound to the dispatcher "active_group"
			// and will be member of active group with name
			// "some_active_group".
			coop->add_agent(
				so_5::rt::agent_ref_t( new a_some_agent_t( env ) ) );
			\endcode
		 */
		agent_coop_unique_ptr_t
		create_coop(
			//! A new cooperation name.
			const nonempty_name_t & name,
			//! A default binder for this cooperation.
			disp_binder_unique_ptr_t disp_binder );

		//! Register a cooperation.
		/*!
		 * The cooperation registration includes following steps:
		 *
		 * - binding agents to the cooperation object;
		 * - checking uniques of the cooperation name. The cooperation will 
		 *   not be registered if its name isn't unique;
		 * - agent_t::so_define_agent() will be called for each agent
		 *   in the cooperation;
		 * - binding of each agent to the dispatcher.
		 *
		 * If all these actions are successful then the cooperation is
		 * marked as registered.
		 *
		 * \retval 0 If cooperation is registered successfully.
		 */
		ret_code_t
		register_coop(
			//! Cooperation to be registered.
			agent_coop_unique_ptr_t agent_coop,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Deregister the cooperation.
		/*!
		 * Method searches the cooperation within registered cooperations and if
		 * it is found deregisters it.
		 *
		 * Deregistration can take some time.
		 *
		 * At first a special signal is sent to cooperation agents.
		 * By receiving these signal agents stop receiving new messages.
		 * When the local event queue for an agent becomes empty the 
		 * agent informs the cooperation about this. When the cooperation 
		 * receives all these signals from agents it informs 
		 * the SObjectizer Run-Time.
		 * Only after this the cooperation is deregistered on the special thread
		 * context.
		 *
		 * After the cooperation deregistration agents are unbound from
		 * dispatchers. And name of the cooperation is removed from
		 * the list of registered cooperations.
		 */
		ret_code_t
		deregister_coop(
			//! Name of the cooperation to be registered.
			const nonempty_name_t & name,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy =
				DO_NOT_THROW_ON_ERROR );

		/*!
		 * \}
		 */

		/*!
		 * \name Methods for working with timer events.
		 * \{
		 */

//FIXME: why not to use a simple std::unique_ptr instead of
//reference and rvalue reference?
		//! Schedule timer event.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Message to be sent after timeout.
			std::unique_ptr< MESSAGE > & msg,
			//! Mbox to which message will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of the delivery repetition for periodic messages.
			/*! 
				\note Value 0 indicates that it's not periodic message (will be delivered one time).. 
			*/
			unsigned int period_msec )
		{
			ensure_message_with_actual_data( msg.get() );

			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Schedule timer event.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Message to be sent after timeout.
			std::unique_ptr< MESSAGE > && msg,
			//! Mbox to which message will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of the delivery repetition for periodic messages.
			/*! 
				\note Value 0 indicates that it's not periodic message (will be delivered one time).. 
			*/
			unsigned int period_msec )
		{
			ensure_message_with_actual_data( msg.get() );

			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Schedule a timer event for a signal.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Mbox to which signal will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of the delivery repetition for periodic messages.
			/*! 
				\note Value 0 indicates that it's not periodic message (will be delivered one time).. 
			*/
			unsigned int period_msec )
		{
			ensure_signal< MESSAGE >();

			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t(),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Schedule a single shot timer event.
		template< class MESSAGE >
		void
		single_timer(
			//! Message to be sent after timeout.
			std::unique_ptr< MESSAGE > msg,
			//! Mbox to which message will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before delivery.
			unsigned int delay_msec )
		{
			ensure_message_with_actual_data( msg.get() );

			single_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec );
		}

		//! Schedule a single shot timer event for a signal.
		template< class MESSAGE >
		void
		single_timer(
			//! Mbox to which signal will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before delivery.
			unsigned int delay_msec )
		{
			ensure_signal< MESSAGE >();

			single_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t(),
				mbox,
				delay_msec );
		}
		/*!
		 * \}
		 */

		/*!
		 * \name Methods for working with layers.
		 * \{
		 */

		//! Get access to the layer.
		template< class SO_LAYER >
		SO_LAYER *
		query_layer(
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR ) const
		{
			// A kind of static assert.
			// Checks that SO_LAYER is derived from the so_layer_t.
			so_layer_t * layer = static_cast< so_layer_t* >( (SO_LAYER *)0 );

			layer = query_layer(
				type_wrapper_t( typeid( SO_LAYER ) ) );

			if( layer )
				return dynamic_cast< SO_LAYER * >( layer );

			if( THROW_ON_ERROR == throwing_strategy )
				throw exception_t(
					"layer does not exist",
					rc_layer_does_not_exist );

			return nullptr;
		}

		//! Add an additional layer.
		template< class SO_LAYER >
		ret_code_t
		add_extra_layer(
			std::unique_ptr< SO_LAYER > layer_ptr,
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			return add_extra_layer(
				type_wrapper_t( typeid( SO_LAYER ) ),
				so_layer_ref_t( layer_ptr.release() ),
				throwing_strategy );
		}
		/*!
		 * \}
		 */

		/*!
		 * \name Methods for starting, initializing and stopping of the Run-Time.
		 * \{
		 */

		//! Run the SObjectizer Run-Time.
		ret_code_t
		run(
			//! Exception strategy.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Initialization hook.
		/*!
		 * \attention A hang inside of this method will prevent the Run-Time
		 * from stopping. For example if a dialog with an application user
		 * is performed inside of init() then SObjectizer cannot finish
		 * its work until this dialog is finished.
		 */
		virtual void
		init() = 0;

		//! Send a shutdown signal to the Run-Time.
		void
		stop();
		/*!
		 * \}
		 */

		//! Access to the environment implementation.
		impl::so_environment_impl_t &
		so_environment_impl();

	private:
		//! Schedule timer event.
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message to be sent after timeout.
			const message_ref_t & msg,
			//! Mbox to which message will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of the delivery repetition for periodic messages.
			/*! 
				\note Value 0 indicates that it's not periodic message (will be delivered one time).. 
			*/
			unsigned int period_msec = 0 );

		//! Schedule a single shot timer event.
		void
		single_timer(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message to be sent after timeout.
			const message_ref_t & msg,
			//! Mbox to which message will be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec );

		//! Access to an additional layer.
		so_layer_t *
		query_layer(
			const type_wrapper_t & type ) const;

		//! Add an additional layer.
		ret_code_t
		add_extra_layer(
			const type_wrapper_t & type,
			const so_layer_ref_t & layer,
			throwing_strategy_t throwing_strategy );

		//! Remove an additional layer.
		ret_code_t
		remove_extra_layer(
			const type_wrapper_t & type,
			throwing_strategy_t throwing_strategy );

		//! Real SObjectizer Environment implementation.
		impl::so_environment_impl_t * m_so_environment_impl;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
