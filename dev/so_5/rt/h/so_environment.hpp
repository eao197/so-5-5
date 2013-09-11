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
#include <so_5/rt/h/mbox_ref.hpp>
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
class message_ref_t;

//
// so_environment_params_t
//

//! Parameters for SObjectizer Environment initialization.
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

		//! Set mutex pool size for syncronizing work with mboxes.
		/*!
		 * A ACE_RW_Thread_Mutex is necessary to work with mboxes.
		 * But because there are could be so many mboxes and lifetime
		 * of them could be so small that it is not efficient to
		 * create a dedicated mutex for every mbox.
		 *
		 * A mutex pool is used by SObjectizer Environment for that
		 * purpose.
		 *
		 * This method allows change the default mutex pool size.
		 *
		 * \see so_environment_t::create_local_mbox().
		 */
		so_environment_params_t &
		mbox_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Set mutex pool size for syncronizing work with agent cooperations.
		/*!
		 * A work with cooperations requires syncronization.
		 * SObjectizer Environment uses pool of mutexes to do that.
		 *
		 * This method allows change the default mutex pool size.
		 */
		so_environment_params_t &
		agent_coop_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Set mutex pool size for syncronizing work with event queues.
		/*!
		 * A work with local agent event queues requires syncronization.
		 * SObjectizer Environment uses pool of mutexes to do that.
		 *
		 * This method allows change the default mutex pool size.
		 */
		so_environment_params_t &
		agent_event_queue_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Add a named dispatcher.
		/*!
		 * By default SObjectizer Environment has only one dispatcher
		 * with one working thread. A user can add his own dispatcher:
		 * named ones.
		 *
		 * \note If a dispatcher with \a name is already registered it
		 * will be replaced by new dispatcher \a dispatcher.
		 */
		so_environment_params_t &
		add_named_dispatcher(
			//! Dispatcher name.
			const nonempty_name_t & name,
			//! Dispatcher.
			dispatcher_unique_ptr_t && dispatcher );

		//! Set timer thread.
		/*!
		 * If \a timer_thread is nullptr then default timer thread
		 * will be used.
		 */
		so_environment_params_t &
		timer_thread(
			//! Timer thread to be set.
			so_5::timer_thread::timer_thread_unique_ptr_t && timer_thread );

		//! Add an additional layer to SObjectize Environment.
		/*!
		 * If that layer is already added it will be replaced by \a layer_ptr.
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
		 * If that layer is already added it will be replaced by \a layer_ptr.
		 */
		void
		add_layer(
			//! Type identification for layer.
			const type_wrapper_t & type,
			//! A layer to be added.
			so_layer_unique_ptr_t layer_ptr );

		//! Size of pool of mutexes to be used with mboxes.
		unsigned int m_mbox_mutex_pool_size;

		//! Size of pool of mutexes to be used with agent cooperations.
		unsigned int m_agent_coop_mutex_pool_size;

		//! Size of pool of mutexes to be used with agent local queues.
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
 * SObjectizer Environment provides basic infrastructure for
 * SObjectizer Run-Time execution.
 *
 * Main method of starting SObjectizer Environment is creating a
 * class derived from so_environment_t and reimplementing
 * so_environment_t::init() method.
 * This method should be used to define starting actions of
 * application. For example first application cooperations could
 * be registered here and starting messages could be sent to them.
 *
 * SObjectizer Environment calls so_environment_t::init() when
 * SObjectize Run-Time is successfully started. If something happened
 * during Run-Time startup then method init() will not be called.
 *
 * SObjectizer Run-Time is started by so_environment_t::run().
 * This method blocks caller thread until SObjectizer completely
 * finished its work.
 *
 * SObjectizer Run-Time is finished by so_environment_t::stop().
 * This method doesn't block caller thread. Instead it sends a special
 * shutdown signal to Run-Time. SObjectizer Run-Time then inform agents
 * about this and waits agents to finishing their work.
 * SObjectizer Run-Time finished when all agents will be stopped and
 * all cooperation will be deregistered.
 *
 * Methods of SObjectizer Environment could be splitted into the
 * following groups:
 * - working with mboxes;
 * - working with dispatchers, exception loggers and handlers;
 * - working with cooperations;
 * - working with delayed and periodic messages;
 * - working with additional layers;
 * - initializing/running/stopping/waiting of Run-Time.
 *
 * \section so_env__mbox_methods Methods for working with mboxes.
 *
 * SObjectizer Environment allows creation of named and anonymous mboxes.
 * A syncronization objects for that mboxes could be obtained from
 * common pools or assigned by user during mbox creation.
 *
 * Mboxes are created by so_environment_t::create_local_mbox() methods.
 * All those methods return mbox_ref_t which is a smart reference to mbox.
 *
 * An anonymous mbox is automatically destroyed when last reference to is is
 * destroyed. So, to save anonymous mbox, the mbox_ref from create_local_mbox()
 * should be stored anywhere.
 *
 * Named mbox should be destroyed manualy by calling
 * so_environment_t::destroy_mbox() method. But physically deletion of
 * named mbox postponed to deletion of last reference to it. So if there is
 * some reference to named mbox it instance will live with that reference.
 * But mbox itself will be removed from SObjectizer Environment lists.
 *
 *
 * \section so_env__coop_methods Methods for working with cooperations.
 *
 * Cooperations could be created by so_environment_t::create_coop() methods.
 *
 * Method so_environment_t::register_coop() should be used for cooperation
 * registration.
 *
 * Method so_environment_t::deregister_coop() should be used for cooperation
 * deregistration.
 *
 * \section so_env__delayed_message_methods Methods for sending delayed and periodic messages.
 *
 * Delayed and/or periodic message is named timer event.
 *
 * Timer event could be created and destroyed. If timer event for
 * delayed message is destroyed before message timeout expired then
 * message delivery canceled. For periodic messages destroying of
 * timer event means that delivery of message will be stopped.
 *
 * Timer events could be created by so_environment_t::schedule_timer()
 * methods. The one version of schedule_timer() is intended for
 * messages with actual data. The second -- for the signals without
 * message data.
 *
 * Methods schedule_timer() return a special reference for timer event.
 * Timer event destroyed when this reference destroyed. So it is necessary
 * to store that reference anywhere. Also timer event could be destroyed
 * by so_5::timer_thread::timer_id_ref_t::release() method.
 *
 * A special method so_environment_t::single_timer() could be used in
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

		//! Create anonymous mbox with default mutex.
		mbox_ref_t
		create_local_mbox();

		//! Create named mbox with default mutex.
		/*!
		 * If \a mbox_name is unique then new mutex is created.
		 * If not the reference to existing mutex is returned.
		 */
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & mbox_name );

		//! Create anonymous mbox with user supplied mutex.
		/*!
		 * Intended for the cases where a dedicated mutex for mbox
		 * is necessary.
		 */
		mbox_ref_t
		create_local_mbox(
			//! A dedicated mutex for mbox.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );

		//! Create named mbox with user supplied mutex.
		/*!
		 * Intended for the cases where a dedicated mutex for mbox
		 * is necessary.
		 */
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & mbox_name,
			//! A dedicated mutex for mbox.
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
		 * \return A reference to dispatcher with name \a disp_name.
		 * Zero reference if dispatcher with such name not found.
		 */
		dispatcher_ref_t
		query_named_dispatcher(
			//! Name of dispatcher.
			const std::string & disp_name );

		//! Set up exception logger.
		void
		install_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		//! Set up exception handler.
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

		//! Create cooperation.
		/*!
		 * \return A new cooperation with \a name. This cooperation
		 * with use default dispatcher binders.
		 */
		agent_coop_unique_ptr_t
		create_coop(
			//! A name for new cooperation.
			const nonempty_name_t & name );

		//! Create cooperation.
		/*!
		 * A binder \a disp_binder will be used for binding cooperation
		 * agents to dispatcher. This binder will be default binder for
		 * that cooperation.
		 *
			\code
			so_5::rt::agent_coop_unique_ptr_t coop = so_env.create_coop(
				so_5::rt::nonempty_name_t( "some_coop" ),
				so_5::disp::active_group::create_disp_binder(
					"active_group",
					"some_active_group" ) );
      
			// That agent will be bound to dispatcher "active_group"
			// and will be member of active group with name
			// "some_active_group".
			coop->add_agent(
				so_5::rt::agent_ref_t( new a_some_agent_t( env ) ) );
			\endcode
		 */
		agent_coop_unique_ptr_t
		create_coop(
			//! A name for new cooperation.
			const nonempty_name_t & name,
			//! A default binder for that cooperation.
			disp_binder_unique_ptr_t disp_binder );

		//! Register cooperation.
		/*!
		 * Cooperation registration includes the following steps:
		 *
		 * - binding agents to cooperation object;
		 * - checking uniquess of cooperation name. Cooperation is
		 *   not registered if its name isn't unique;
		 * - agent_t::so_define_agent() is called for each agent
		 *   in cooperation;
		 * - binding of each agent to dispatcher.
		 *
		 * If all those actions are successful then cooperation is
		 * marked as registered.
		 *
		 * \retval 0 If cooperation registered successfully.
		 */
		ret_code_t
		register_coop(
			//! Cooperation to be registered.
			agent_coop_unique_ptr_t agent_coop,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Deregister cooperation.
		/*!
		 * Searches cooperation between registered cooperations and if
		 * it is found deregistes it.
		 *
		 * Deregistration could take some time.
		 *
		 * At first a special signal is sent to cooperation agents.
		 * By receiving that signal agents stops receiving new messages.
		 * When local event queue for agent becomes empty agent informs
		 * cooperation about this. When cooperation receives all
		 * those signals from agents it informs SObjectizer Run-Time.
		 * Only after that cooperation is deregistered on special thread
		 * context.
		 *
		 * After cooperation deregistration agents are unbound from
		 * dispatchers. And name of cooperation is removed from
		 * list of registered cooperations.
		 */
		ret_code_t
		deregister_coop(
			//! Name of cooperation to be registered.
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
			//! Mbox to which message should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of delivery repetition for periodic messages.
			//! Value 0 indicates delayed only message.
			unsigned int period_msec )
		{
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
			//! Mbox to which message should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of delivery repetition for periodic messages.
			//! Value 0 indicates delayed only message.
			unsigned int period_msec )
		{
			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Schedule timer event for a signal.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Mbox to which signal should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of delivery repetition for periodic signals.
			//! Value 0 indicates delayed only signal.
			unsigned int period_msec )
		{
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
			//! Mbox to which message should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before delivery.
			unsigned int delay_msec )
		{
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
			//! Mbox to which signal should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before delivery.
			unsigned int delay_msec )
		{
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

		//! Get access to layer.
		template< class SO_LAYER >
		SO_LAYER *
		query_layer(
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR ) const
		{
			// A kind of static assert.
			// Checks that SO_LAYER is derived from so_layer_t.
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
		 * \name Methods for starting, initializing and stopping of Run-Time.
		 * \{
		 */

		//! Run SObjectizer Run-Time.
		ret_code_t
		run(
			//! Exception strategy.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Initialization hook.
		/*!
		 * \attention A hang inside this method will prevent Run-Time
		 * from stopping. For example if a dialog with application user
		 * is performed inside init() then SObjectizer cannot finish
		 * its work until this dialog is finished.
		 */
		virtual void
		init() = 0;

		//! Send a shutdown signal to Run-Time.
		void
		stop();
		/*!
		 * \}
		 */

		//! Access to environment implementation.
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
			//! Mbox to which message should be delivered.
			const mbox_ref_t & mbox,
			//! Timeout before the first delivery.
			unsigned int delay_msec,
			//! Period of delivery repetition for periodic messages.
			//! Value 0 indicates delayed only message.
			unsigned int period_msec = 0 );

		//! Schedule a single shot timer event.
		void
		single_timer(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message to be sent after timeout.
			const message_ref_t & msg,
			//! Mbox to which message should be delivered.
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
