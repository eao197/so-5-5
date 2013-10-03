/*
	SObjectizer 5.
*/

#include <ace/Guard_T.h>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

#include <so_5/rt/impl/h/cmp_method_ptr.hpp>
#include <so_5/rt/impl/h/message_consumer_link.hpp>
#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/local_event_queue.hpp>
#include <so_5/rt/impl/h/void_dispatcher.hpp>
#include <so_5/rt/impl/h/state_listener_controller.hpp>

namespace so_5
{

namespace rt
{

namespace /* ananymous */
{

//
// hook_event_caller_t
//

//! The base class for starting and finishing event calling.
class hook_event_caller_t
	:
		public event_handler_caller_t
{
	public:

		hook_event_caller_t(
			agent_t & agent )
			:
				m_agent( agent )
		{}

		virtual ~hook_event_caller_t()
		{}

		/*!
		 * \name Inherited methods implementation.
		 * \{
		 */

		virtual type_wrapper_t
		type_wrapper() const
		{
			// Should never be called for this class.
			std::abort();
			type_wrapper_t fake( typeid( message_t ) );
			return fake;
		};
		/*!
		 * \}
		 */

	protected:
		agent_t & m_agent;
};

//
// start_hook_event_caller_t
//

/*!
 * \brief Hook for the starting event.
 *
 * That event is added into the event queue as first event to
 * call so_5::rt::so_evt_start().
 */
class start_hook_event_caller_t
	:
		public hook_event_caller_t
{
	public:

		start_hook_event_caller_t(
			agent_t & agent )
			:
				hook_event_caller_t( agent )
		{}
		virtual ~start_hook_event_caller_t()
		{}

		/*!
		 * \name Inherited methods implementation.
		 * \{
		 */

		virtual bool
		call(
			message_ref_t & ) const
		{
			m_agent.so_evt_start();
			return true;
		}
		/*!
		 * \}
		 */
};

//
// finish_hook_event_caller_t
//


/*!
 * \brief Hook for the starting event.
 *
 * This event is used to call the so_5::rt::so_evt_start().
 */
class finish_hook_event_caller_t
	:
		public hook_event_caller_t
{
	public:

		finish_hook_event_caller_t(
			agent_t & agent )
			:
				hook_event_caller_t( agent )
		{}
		virtual ~finish_hook_event_caller_t()
		{}

		/*!
		 * \name Inherited methods implementation.
		 * \{
		 */
		virtual bool
		call(
			message_ref_t & ) const
		{
			m_agent.so_evt_finish();
			return true;
		}
		/*!
		 * \}
		 */
};

/*!
 * A stub for the initial value of the so_5::rt::agent_t::m_dispatcher.
 */
impl::void_dispatcher_t g_void_dispatcher;

}/* ananymous namespace */

//
// agent_t
//

agent_t::agent_t(
	so_environment_t & env )
	:
		m_default_state( self_ptr() ),
		m_current_state_ptr( &m_default_state ),
		m_was_defined( false ),
		m_state_listener_controller(
			new impl::state_listener_controller_t ),
		m_so_environment_impl( 0 ),
		m_dispatcher( &g_void_dispatcher ),
		m_agent_coop( 0 ),
		m_is_coop_deregistered( false )
{
	// Bind to the environment should be done.
	bind_to_environment( env.so_environment_impl() );
}

agent_t::~agent_t()
{
}

const agent_t *
agent_t::self_ptr() const
{
	return this;
}

void
agent_t::so_evt_start()
{
	// Default implementation do nothing.
}

void
agent_t::so_evt_finish()
{
	// Default implementation do nothing.
}


const std::string &
agent_t::so_coop_name() const
{
	if( 0 == m_agent_coop )
		throw exception_t(
			"agent isn't bound to cooperation yet",
			rc_agent_has_no_cooperation );

	return m_agent_coop->query_coop_name();
}

void
agent_t::so_add_nondestroyable_listener(
	agent_state_listener_t & state_listener )
{
	m_state_listener_controller->so_add_nondestroyable_listener(
		state_listener );
}

void
agent_t::so_add_destroyable_listener(
	agent_state_listener_unique_ptr_t state_listener )
{
	m_state_listener_controller->so_add_destroyable_listener(
		std::move( state_listener ) );
}

const state_t &
agent_t::so_default_state() const
{
	return m_default_state;
}

void
agent_t::so_change_state(
	const state_t & new_state )
{
	if( new_state.is_target( this ) )
	{
		m_current_state_ptr = &new_state;

		// State listener should be informed.
		m_state_listener_controller->changed(
			*this,
			*m_current_state_ptr );
	}
	else
		SO_5_THROW_EXCEPTION(
			rc_agent_unknown_state,
			"unable to switch agent to alien state "
			"(the state that doesn't belong to this agent)" );
}

void
agent_t::so_define_agent()
{
	// Default implementation do nothing.
}

bool
agent_t::so_was_defined() const
{
	return m_was_defined;
}

void
agent_t::define_agent()
{
	so_define_agent();
	m_was_defined = true;
}

void
agent_t::undefine_agent()
{
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

		m_is_coop_deregistered = true;

		if( m_dispatcher != &g_void_dispatcher )
		{
			// A final event handler should be added.
			event_handler_caller_ref_t evt_caller(
				new finish_hook_event_caller_t( *this ) );

			event_caller_block_ref_t event_caller_block(
				new event_caller_block_t );

			event_caller_block->insert( evt_caller );

			m_local_event_queue->push(
				impl::event_item_t(
					event_caller_block,
					message_ref_t() ) );

			// Dispatcher should be informed about this event.
			m_dispatcher->put_event_execution_request(
				create_ref(), 1 );
		}
		else
		{
			// There is no the real dispatcher.
			// So all events could be simple removed.
			m_local_event_queue->clear();
		}
	}

	// Subscriptions should be destroyed.
	destroy_all_subscriptions();
}

so_environment_t &
agent_t::so_environment()
{
	return m_so_environment_impl->query_public_so_environment();
}

agent_ref_t
agent_t::create_ref()
{
	agent_ref_t agent_ref( this );
	return agent_ref;
}

void
agent_t::bind_to_coop(
	agent_coop_t & coop )
{
	m_agent_coop = &coop;
	m_is_coop_deregistered = false;
}

inline void
agent_t::bind_to_environment(
	impl::so_environment_impl_t & env_impl )
{
	m_so_environment_impl = &env_impl;
	m_local_event_queue = m_so_environment_impl->create_local_queue();

	// A staring event should be placed at begining of the queue.
	event_handler_caller_ref_t evt_caller(
		new start_hook_event_caller_t( *this ) );

	event_caller_block_ref_t event_caller_block(
		new event_caller_block_t );

	event_caller_block->insert( evt_caller );

	m_local_event_queue->push(
		impl::event_item_t(
			event_caller_block,
			message_ref_t() ) );
}

void
agent_t::bind_to_disp(
	dispatcher_t & disp )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	// A pointer to the stub should be in the m_dispatcher.
	// If it is not true then agent is already bound to the dispatcher.
	if( m_dispatcher != &g_void_dispatcher )
	{
		throw exception_t(
			"agent is already bind to dispatcher",
			rc_agent_is_already_bind_to_disp );
	}

	m_dispatcher = &disp;

	// Dispatcher should be informed about events in the local queue.
	m_dispatcher->put_event_execution_request(
		create_ref(),
		m_local_event_queue->size() );
}

//! Make textual representation of the subscription key.
template< class PAIR >
inline std::string
subscription_key_string( const PAIR & sk )
{
	const std::string msg_type =
		sk.first.query_type_info().name();

	std::string str = "message type: " +
		msg_type + "; mbox: " + sk.second->query_name();

	return str;
}

void
agent_t::create_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc )
{
	subscription_key_t subscr_key( type_wrapper, mbox_ref );

	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	if( m_is_coop_deregistered )
		return;

	consumers_map_t::iterator it = m_event_consumers_map.find( subscr_key );

	// If subscription is not found then it should be created.
	if( m_event_consumers_map.end() == it )
	{
		impl::message_consumer_link_t * message_consumer_link_ptr =
			new impl::message_consumer_link_t( create_ref() );

		mbox_ref->subscribe_first_event_handler(
			type_wrapper,
			std::unique_ptr< impl::message_consumer_link_t >(
				message_consumer_link_ptr ),
			ehc );

		m_event_consumers_map.insert(
			consumers_map_t::value_type(
				subscr_key,
				message_consumer_link_ptr ) );
	}
	else
	{
		mbox_ref->subscribe_more_event_handler(
			type_wrapper,
			it->second,
			ehc );
	}
}

void
agent_t::destroy_all_subscriptions()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	consumers_map_t::iterator it = m_event_consumers_map.begin();
	consumers_map_t::iterator it_end = m_event_consumers_map.end();

	for(; it != it_end; ++it )
	{
		mbox_ref_t mbox( it->first.second );
		mbox->unsubscribe_event_handlers(
			it->first.first,
			it->second );
	}
	m_event_consumers_map.clear();
}

void
agent_t::push_event(
	const event_caller_block_ref_t & event_caller_block,
	const message_ref_t & message )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	// Event can be added only if the agent is not deregistered yet.
	if( !m_is_coop_deregistered )
	{
		m_local_event_queue->push(
			impl::event_item_t( event_caller_block, message ) );

		m_dispatcher->put_event_execution_request(
			create_ref(), 1 );
	}
}

void
agent_t::exec_next_event()
{
	impl::event_item_t event_item;
	bool agent_finished_his_work_totally = false;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

		m_local_event_queue->pop( event_item );

		// This value is necessary to inform the cooperation about agent
		// work finishing in case of the cooperation deregistration.
		agent_finished_his_work_totally =
			m_is_coop_deregistered && 0 == m_local_event_queue->size();
	}

	event_item.m_event_caller_block->call(
		event_item.m_message_ref );

	// Special case for the cooperation deregistration.
	if( agent_finished_his_work_totally )
	{
		agent_coop_t::call_agent_finished( *m_agent_coop );
	}
}

} /* namespace rt */

} /* namespace so_5 */

