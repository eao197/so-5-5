/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A base class for agents.
*/

#if !defined( _SO_5__RT__AGENT_HPP_ )
#define _SO_5__RT__AGENT_HPP_

#include <memory>
#include <map>

#include <so_5/h/declspec.hpp>
#include <so_5/h/types.hpp>

#include <so_5/h/exception.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/agent_ref_fwd.hpp>
#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/event_caller_block.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/agent_state_listener.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

// Forward declarations.
class local_event_queue_t;
class message_consumer_link_t;
class so_environment_impl_t;
class state_listener_controller_t;

} /* namespace impl */

class state_t;
class so_environment_t;
class agent_coop_t;
class agent_t;

//
// exception_reaction_t
//
/*!
 * \since v.5.2.3
 * \brief A reaction of SObjectizer to an exception from agent event.
 */
enum exception_reaction_t
{
	//! Execution of application must be aborted immediatelly.
	abort_on_exception = 1,
	//! Agent must be switched to special state and SObjectizer
	//! Environment will be stopped.
	shutdown_sobjectizer_on_exception = 2,
	//! Agent must be switched to special state and agent's cooperation
	//! must be deregistered.
	deregister_coop_on_exception = 3,
	//! Exception should be ignored and agent should continue its work.
	ignore_exception = 4
};

//
// subscription_bind_t
//

/*!
 * \brief A class for creating a subscription to messages from the mbox.
*/
class subscription_bind_t
{
	public:
		inline
		subscription_bind_t(
			//! Agent to subscribe.
			agent_t & agent,
			//! Mbox for messages to be subscribed.
			const mbox_ref_t & mbox_ref );

		//! Set up a state in which events are allowed be processed.
		inline subscription_bind_t &
		in(
			//! State in which events are allowed.
			const state_t & state );

		//! Make subscription to the message.
		template< class MESSAGE, class AGENT >
		void
		event(
			//! Event handling method.
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) );

	private:
		//! Agent to which we are subscribing.
		agent_t & m_agent;
		//! Mbox for messages to subscribe.
		mbox_ref_t m_mbox_ref;
		//! State for events.
		const state_t * m_state;
};

//
// agent_t
//

//! A base class for agents.
/*!
	An agent in SObjctizer must be derived from the agent_t.

	The base class provides various methods whose can be splitted into
	the following groups:
	\li methods for the interaction with SObjectizer;
	\li predefined hook-methods which are called during: cooperation
	registration, starting and stopping of an agent;
	\li methods for the message subscription and unsubscription;
	\li methods for working with an agent state;

	<b>Methods for the interaction with SObjectizer</b>

	Method so_5::rt::agent_t::so_environment() serves for the access to the 
	SObjectizer Environment (and, therefore, to all methods of the 
	SObjectizer Environment).
	This method could be called immediatelly after the agent creation.
	This is because agent is bound to the SObjectizer Environment during
	the creation process.

	<b>Hook methods</b>

	The base class defines several hook-methods. Its default implementation
	do nothing.

	The method agent_t::so_define_agent() is called just before agent will
	started by SObjectizer as a part of the agent registration process.
	It should be reimplemented for the initial subscription of the agent
	to messages.

	There are two hook-methods related to important agent's lifetime events:
	agent_t::so_evt_start() and agent_t::so_evt_finish(). They are called
	by SObjectizer in next circumstances:
	- method so_evt_start() is called when the agent is starting its work
	  inside of SObjectizer. At that moment all agents are defined (all 
	  their methods agent_t::so_define_agent() have executed);
	- method so_evt_finish() is called during the agent's cooperation
	  deregistration just after agent processed the last pending event.

	Methods so_evt_start() and so_evt_finish() are called by SObjectizer and
	user can just reimplement them to implement the agent-specific logic.

	<b>Message subscription and unsubscription methods</b>

	Any method with the following prototype can be used as an event
	handler:
	\code
		void
		evt_handler(
			const so_5::rt::event_data_t< MESSAGE > & msg );
	\endcode
	Where \c evt_handler is a name of the event handler, \c MESSAGE is a 
	message type.

	The class so_5::rt::event_data_t is a wrapper on pointer to an instance 
	of the \c MESSAGE. It is very similar to <tt>std::unique_ptr</tt>. 
	The pointer to \c MESSAGE can be a nullptr. It happens in case when 
	the message has no actual data and servers just a signal about something.

	A subscription to the message is performed by the method so_subscribe().
	This method returns an instance of the so_5::rt::subscription_bind_t which
	does all actual actions of the subscription process. This instance already
	knows agents and message mbox and uses the default agent state for
	the event subscription (binding to different state is also possible). 

	<b>Methods for working with an agent state</b>

	The agent can change its state by his so_change_state() method.

	An attempt to switch an agent to the state which belongs to the another
	agent is an error. If state is belong to the same agent there are
	no possibility to any run-time errors. In this case changing agent
	state is a very safe operation.

	In some cases it is necessary to detect agent state switching.
	For example for application monitoring purposes. This can be done
	by "state listeners".

	Any count of state listeners can be set for an agent. There are
	two methods for that:
	- so_add_nondestroyable_listener() is for listeners whose lifetime
	  are controlled by a programmer, not by SObjectizer;
	- so_add_destroyable_listener() is for listeners whose lifetime
	  must be controlled by agent itself.
*/
class SO_5_TYPE agent_t
	:
		private atomic_refcounted_t
{
		friend class subscription_bind_t;
		friend class smart_atomic_reference_t< agent_t >;
		friend class agent_coop_t;

	public:
		//! Constructor.
		/*!
			Agent must be bound to the SObjectizer Environment during
			its creation. And that binding cannot be changed anymore.
		*/
		explicit agent_t(
			//! The Environment for this agent must exist.
			so_environment_t & env );

		virtual ~agent_t();

		//! Get the raw pointer of itself.
		/*!
			This method is intended for use in the member initialization
			list instead 'this' to suppres compiler warnings.
			For example for an agent state initialization:
			\code
			class a_sample_t
				:
					public so_5::rt::agent_t
			{
					typedef so_5::rt::agent_t base_type_t;

					// Agent state.
					const so_5::rt::state_t m_sample_state;
				public:
					a_sample_t( so_5::rt::so_environment_t & env )
						:
							base_type_t( env ),
							m_sample_state( self_ptr() )
					{
						// ...
					}

				// ...

			};
			\endcode
		*/
		const agent_t *
		self_ptr() const;

		//! Hook on agent start inside SObjectizer.
		/*!
			It is guaranteed that this method will be called first
			just after end of the cooperation registration process.

			During cooperation registration agent is bound to some
			working thread. And the first method which is called for
			the agent on that working thread context is this method.

			\code
			class a_sample_t
				:
					public so_5::rt::agent_t
			{
				// ...
				virtual void
				so_evt_start();
				// ...
			};

			a_sample_t::so_evt_start()
			{
				std::cout << "first agent action on bound dispatcher" << std::endl;
				... // Some application logic actions.
			}
			\endcode
		*/
		virtual void
		so_evt_start();

		//! Hook of agent finish in SObjectizer.
		/*!
			It is guaranteed that this method will be called last
			just before deattaching agent from it's working thread.

			This method should be used to perform some cleanup
			actions on it's working thread.
			\code
			class a_sample_t
				:
					public so_5::rt::agent_t
			{
				// ...
				virtual void
				so_evt_finish();
				// ...
			};

			a_sample_t::so_evt_finish()
			{
				std::cout << "last agent activity";

				if( so_current_state() == m_db_error_happened )
				{
					// Delete the DB connection on the same thread where
					// connection was established and where some
					// error happened.
					m_db.release();
				}
			}
			\endcode
		*/
		virtual void
		so_evt_finish();

		//! Access to the current agent state.
		inline const state_t &
		so_current_state() const
		{
			return *m_current_state_ptr;
		}

		//! Name of the agent's cooperation.
		/*!
		 * \note It is safe to use this method when agent is working inside 
		 * SObjectizer because agent can be registered only in some
		 * cooperation. When agent is not registered in SObjectizer this
		 * method should be used carefully.
		 *
		 * \throw so_5::exception_t If the agent doesn't belong to any cooperation.
		 *
		 * \return Cooperation name if the agent is bound to the cooperation.
		 */
		const std::string &
		so_coop_name() const;

		//! Add a state listener to the agent.
		/*!
		 * A programmer should guarantee that the lifetime of
		 * \a state_listener is exceeds lifetime of the agent.
		 */
		void
		so_add_nondestroyable_listener(
			agent_state_listener_t & state_listener );

		//! Add a state listener to the agent.
		/*!
		 * Agent takes care of the \a state_listener destruction.
		 */
		void
		so_add_destroyable_listener(
			agent_state_listener_unique_ptr_t state_listener );

		/*!
		 * \since v.5.2.3.
		 * \brief A reaction from SObjectizer to an exception from
		 * agent's event.
		 *
		 * If an exception is going out from agent's event it will be
		 * caught by SObjectizer. Then SObjectizer will call this method
		 * and perform some actions in dependence of return value.
		 *
		 * By default this method returns deregister_coop_on_exception.
		 */
		virtual exception_reaction_t
		so_exception_reaction() const;

		/*!
		 * \since 5.2.3
		 * \brief Switching agent to special state in case of unhandled
		 * exception.
		 */
		void
		so_switch_to_awaiting_deregistration_state();

		//! Push an event to the agent's event queue.
		/*!
			This method is used by SObjectizer for the 
			agent's event scheduling.
		*/
		static inline void
		call_push_event(
			agent_t & agent,
			const event_caller_block_t * event_handler_caller,
			const message_ref_t & message )
		{
			agent.push_event( event_handler_caller, message );
		}

		//! Run the event handler for the next event.
		/*!
			This method is used by a dispatcher/working thread for
			the event handler execution.
		*/
		static inline void
		call_next_event(
			//! Agent which event will be executed.
			agent_t & agent )
		{
			agent.exec_next_event();
		}

		//! Bind agent to the dispatcher.
		static inline void
		call_bind_to_disp(
			agent_t & agent,
			dispatcher_t & disp )
		{
			agent.bind_to_disp( disp );
		}

	protected:
		/*!
		 * \name Methods for working with the agent state.
		 * \{
		 */

		//! Access to the agent's default state.
		const state_t &
		so_default_state() const;

		//! Method changes state.
		/*!
			Usage sample:
			\code
			void
			a_sample_t::evt_smth(
				const so_5::rt::event_data_t< message_one_t > & msg )
			{
				// If something wrong with the message then we should
				// switch to the error_state.
				if( error_in_data( *msg ) )
					so_change_state( m_error_state );
			}
			\endcode
		*/
		void
		so_change_state(
			//! New agent state.
			const state_t & new_state );
		/*!
		 * \}
		 */

	public : /* Note: since v.5.2.3.2 subscription-related method are
					made public. */

		/*!
		 * \name Subscription methods.
		 * \{
		 */

		//! Initiate subscription.
		/*!
			Usage sample:
			\code
			void
			a_sample_t::so_define_agent()
			{
				so_subscribe( m_mbox_target )
					.in( m_state_one )
						.event( &a_sample_t::evt_sample_handler );
			}
			\endcode
		*/
		inline subscription_bind_t
		so_subscribe(
			//! Mbox for messages to subscribe.
			const mbox_ref_t & mbox_ref )
		{
			return subscription_bind_t( *this, mbox_ref );
		}

		/*!
		 * \since v.5.2.3
		 * \brief Drop subscription for the state specified.
		 *
		 * \note Doesn't throw if there is no such subscription.
		 *
		 * \note Subscription is removed even if agent was subscribed
		 * for this message type with different method pointer.
		 * The pointer to event routine is necessary only to
		 * detect MSG type.
		 */
		template< class AGENT, class MESSAGE >
		inline void
		so_drop_subscription(
			const mbox_ref_t & mbox,
			const state_t & target_state,
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) )
		{
			do_drop_subscription( typeid( MESSAGE ), mbox, target_state );
		}

		/*!
		 * \since v.5.2.3
		 * \brief Drop subscription for the default agent state.
		 *
		 * \note Doesn't throw if there is no such subscription.
		 *
		 * \note Subscription is removed even if agent was subscribed
		 * for this message type with different method pointer.
		 * The pointer to event routine is necessary only to
		 * detect MSG type.
		 */
		template< class AGENT, class MESSAGE >
		inline void
		so_drop_subscription(
			const mbox_ref_t & mbox,
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) )
		{
			do_drop_subscription( typeid( MESSAGE ), mbox, so_default_state() );
		}

		/*!
		 * \since v.5.2.3
		 * \brief Drop subscription for all states.
		 *
		 * \note Doesn't throw if there is no any subscription for
		 * that mbox and message type.
		 *
		 * \note Subscription is removed even if agent was subscribed
		 * for this message type with different method pointer.
		 * The pointer to event routine is necessary only to
		 * detect MSG type.
		 */
		template< class AGENT, class MESSAGE >
		inline void
		so_drop_subscription_for_all_states(
			const mbox_ref_t & mbox,
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) )
		{
			do_drop_subscription_for_all_states( typeid( MESSAGE ), mbox );
		}
		/*!
		 * \}
		 */

	protected :

		/*!
		 * \name Agent initialization methods.
		 * \{
		 */

		//! Hook on define agent for SObjectizer.
		/*!
			This method is called by SObjectizer during the cooperation
			registration process before agent will be bound to its
			working thread.

			Should be used by the agent to make necessary message subscriptions.

			Usage sample;
			\code
			class a_sample_t
				:
					public so_5::rt::agent_t
			{
				// ...
				virtual void
				so_define_agent();

				void
				evt_handler_1(
					const so_5::rt::event_data_t< message1_t > & msg );
				// ...
				void
				evt_handler_N(
					const so_5::rt::event_data_t< messageN_t > & msg );

			};

			void
			a_sample_t::so_define_agent()
			{
				// Make subscriptions...
				so_subscribe( m_mbox1 )
					.in( m_state_1 )
						.event( &a_sample_t::evt_handler_1 );
				// ...
				so_subscribe( m_mboxN )
					.in( m_state_N )
						.event( &a_sample_t::evt_handler_N );
			}
			\endcode
		*/
		virtual void
		so_define_agent();

		//! Is method define_agent already called?
		/*!
			Usage sample:
			\code
			class a_sample_t
				:
					public so_5::rt::agent_t
			{
				// ...

				public:
					void
					set_target_mbox(
						const so_5::rt::mbox_ref_t & mbox )
					{
						// mbox cannot be changed after agent registration.
						if( !so_was_defined() )
						{
							m_target_mbox = mbox;
						}
					}

				private:
					so_5::rt::mbox_ref_t m_target_mbox;
			};
			\endcode
		*/
		bool
		so_was_defined() const;
		/*!
		 * \}
		 */

	public:
		//! Access to the SObjectizer Environment which this agent is belong.
		/*!
			Usage sample for other cooperation registration:
			\code
			void
			a_sample_t::evt_on_smth(
				const so_5::rt::event_data_t< some_message_t > & msg )
			{
				so_5::rt::agent_coop_unique_ptr_t coop =
					so_environment().create_coop(
						so_5::rt::nonempty_name_t( "first_coop" ) );

				// Filling the cooperation...
				coop->add_agent( so_5::rt::agent_ref_t(
					new a_another_t( ... ) ) );
				// ...

				// Registering cooperation.
				so_environment().register_coop( coop );
			}
			\endcode

			Usage sample for the SObjectizer shutting down:
			\code
			void
			a_sample_t::evt_last_event(
				const so_5::rt::event_data_t< message_one_t > & msg )
			{
				...
				so_environment().stop();
			}
			\endcode
		*/
		so_environment_t &
		so_environment();

	private:
		//! Default agent state.
		const state_t m_default_state;

		//! Current agent state.
		const state_t * m_current_state_ptr;

		/*!
		 * \since v.5.2.3
		 * \brief A special state for awaiting deregistration.
		 *
		 * Agent is switched to that state when it let an exception to go
		 * out from event handler.
		 */
		const state_t m_awaiting_deregistration_state;

		//! Agent definition flag.
		/*!
		 * Set to true after a successful return from the so_define_agent().
		 */
		bool m_was_defined;

		//! State listeners controller.
		std::unique_ptr< impl::state_listener_controller_t >
			m_state_listener_controller;

		//! Typedef for subscription key.
		typedef std::pair< type_wrapper_t, mbox_ref_t > subscription_key_t;

		//! Typedef for the map from subscriptions to event handlers.
		typedef std::map<
				subscription_key_t,
				event_caller_block_ref_t >
			consumers_map_t;

		//! Map from subscriptions to event handlers.
		consumers_map_t m_event_consumers_map;

		//! Local events queue.
		std::unique_ptr< impl::local_event_queue_t >
			m_local_event_queue;

		//! SObjectizer Environment for which the agent is belong.
		impl::so_environment_impl_t * m_so_environment_impl;

		//! Dispatcher of this agent.
		/*!
		 * By default this pointer points to a special stub.
		 * This stub do nothing but allows safely call the method for
		 * events scheduling.
		 *
		 * This pointer received the actual value after binding
		 * agent to the real dispatcher.
		 */
		dispatcher_t * m_dispatcher;

		//! Agent is belong to this cooperation.
		agent_coop_t * m_agent_coop;

		//! Is the cooperation deregistration in progress?
		bool m_is_coop_deregistered;

		//! Make an agent reference.
		/*!
		 * This is an internal SObjectizer method. It is called when
		 * it is guaranteed that the agent is still necessary and something
		 * has reference to it.
		 */
		agent_ref_t
		create_ref();

		/*!
		 * \name Embedding agent into the SObjectizer Run-time.
		 * \{
		 */

		//! Bind agent to the cooperation.
		/*!
		 * Initializes an internal cooperation pointer.
		 *
		 * Drops m_is_coop_deregistered to false.
		 */
		void
		bind_to_coop(
			//! Cooperation for that agent.
			agent_coop_t & coop );

		//! Bind agent to the SObjectizer Environment.
		/*!
		 * Method is called from the agent constructor.
		 *
		 * Method initializes the internal SObjectizer Environment pointer.
		 */
		void
		bind_to_environment(
			impl::so_environment_impl_t & env_impl );

		//! Bind agent to the dispatcher.
		/*!
		 * Method initializes the internal dispatcher poiner.
		 */
		void
		bind_to_disp(
			dispatcher_t & disp );

		/*!
		 * \since v.5.2.3
		 * \brief Start agent work.
		 *
		 * This method is called after all registration specific actions.
		 *
		 * Method checks the local event queue. If the queue is not empty then
		 * method tells to dispatcher to schedule the agent 
		 * for the event processing.
		 */
		void
		start_agent();

		//! Agent definition driver.
		/*!
		 * Method calls so_define_agent() and then stores an agent
		 * definition flag.
		 */
		void
		define_agent();

		//! Agent shutdown deriver.
		/*!
		 * \since v.5.2.3
		 *
		 * Method destroys all agent subscriptions.
		 */
		void
		shutdown_agent();
		/*!
		 * \}
		 */

		/*!
		 * \name Subscription/unsubscription implementation details.
		 * \{
		 */

		//! Create binding between agent and mbox.
		void
		create_event_subscription(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message's mbox.
			mbox_ref_t & mbox_ref,
			//! State for event.
			const state_t & target_state,
			//! Event handler caller.
			const event_handler_caller_ref_t & ehc );

		/*!
		 * \since v.5.2.0
		 * \brief Create and register event caller block for new subcription.
		 */
		void
		create_and_register_event_caller_block(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message's mbox.
			mbox_ref_t & mbox_ref,
			//! State for event.
			const state_t & target_state,
			//! Event handler caller.
			const event_handler_caller_ref_t & ehc,
			//! Subscription key for that event.
			const subscription_key_t & subscr_key );

		//! Destroy all agent subscriptions.
		void
		destroy_all_subscriptions();

		/*!
		 * \since v.5.2.0
		 * \brief Clean event consumers map.
		 */
		void
		clean_consumers_map();

		/*!
		 * \since v.5.2.3
		 * \brief Remove subscription for the state specified.
		 */
		void
		do_drop_subscription(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message's mbox.
			const mbox_ref_t & mbox_ref,
			//! State for event.
			const state_t & target_state );

		/*!
		 * \since v.5.2.3
		 * \brief Remove subscription for all states.
		 */
		void
		do_drop_subscription_for_all_states(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message's mbox.
			const mbox_ref_t & mbox_ref );
		/*!
		 * \}
		 */

		/*!
		 * \name Event handling implementation details.
		 * \{
		 */

		//! Push event into the local event queue.
		void
		push_event(
			//! Event handler caller for an event.
			const event_caller_block_t * event_handler_caller,
			//! Event message.
			const message_ref_t & message );

		//! Execute the next event.
		/*!
		 * \attention Must be called only on working thread context.
		 *
		 * \pre The local event queue must be not empty.
		 */
		void
		exec_next_event();
		/*!
		 * \}
		 */
		/*!
		 * \name Demand handlers.
		 * \{
		 */
		/*!
		 * \since v.5.2.0
		 * \brief Calls so_evt_start method for agent.
		 */
		static void
		demand_handler_on_start(
			message_ref_t &,
			const event_caller_block_t *,
			agent_t * agent );
		/*!
		 * \since v.5.2.0
		 * \brief Calls so_evt_finish method for agent.
		 */
		static void
		demand_handler_on_finish(
			message_ref_t &,
			const event_caller_block_t *,
			agent_t * agent );
		/*!
		 * \since v.5.2.0
		 * \brief Calls event handler for message.
		 */
		static void
		demand_handler_on_message(
			message_ref_t & msg,
			const event_caller_block_t * event_handler,
			agent_t * );
		/*!
		 * \}
		 */
};

//
// subscription_bind_t implementation
//
inline
subscription_bind_t::subscription_bind_t(
	agent_t & agent,
	const mbox_ref_t & mbox_ref )
	:	m_agent( agent )
	,	m_mbox_ref( mbox_ref )
	,	m_state( &m_agent.so_default_state() )
{
}

inline subscription_bind_t &
subscription_bind_t::in(
	const state_t & state )
{
	if( !state.is_target( &m_agent ) )
	{
		SO_5_THROW_EXCEPTION(
			rc_agent_is_not_the_state_owner,
			"agent doesn't own the state" );
	}

	m_state = &state;

	return *this;
}

template< class MESSAGE, class AGENT >
inline void
subscription_bind_t::event(
	void (AGENT::*pfn)( const event_data_t< MESSAGE > & ) )
{
	// Agent must have right type.
	AGENT * cast_result = dynamic_cast< AGENT * >( &m_agent );

	// Was conversion successful?
	if( nullptr == cast_result )
	{
		// No. Actual type of the agent is not convertible to the AGENT.
		SO_5_THROW_EXCEPTION(
			rc_agent_incompatible_type_conversion,
			std::string( "Unable convert agent to type: " ) +
				typeid(AGENT).name() );
	}

	m_agent.create_event_subscription(
		typeid( MESSAGE ),
		m_mbox_ref,
		*m_state,
		event_handler_caller_ref_t(
			new real_event_handler_caller_t< MESSAGE, AGENT >(
				pfn,
				*cast_result ) ) );
}

} /* namespace rt */

} /* namespace so_5 */

#endif

