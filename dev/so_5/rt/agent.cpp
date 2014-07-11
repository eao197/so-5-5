/*
	SObjectizer 5.
*/

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/state_listener_controller.hpp>

namespace so_5
{

namespace rt
{

//
// agent_t
//

agent_t::agent_t(
	so_environment_t & env )
	:	m_default_state( self_ptr() )
	,	m_current_state_ptr( &m_default_state )
	,	m_awaiting_deregistration_state( self_ptr() )
	,	m_was_defined( false )
	,	m_state_listener_controller( new impl::state_listener_controller_t )
	,	m_so_environment_impl( 0 )
	,	m_tmp_event_queue( m_mutex )
		// It is necessary to enable agent subscription in the
		// constructor of derived class.
	,	m_working_thread_id( std::this_thread::get_id() )
	,	m_agent_coop( 0 )
	,	m_is_coop_deregistered( false )
{
	m_event_queue = &m_tmp_event_queue;

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

exception_reaction_t
agent_t::so_exception_reaction() const
{
	if( m_agent_coop )
		return m_agent_coop->exception_reaction();
	else
		// This is very strange case. So it would be better to abort.
		return abort_on_exception;
}

void
agent_t::so_switch_to_awaiting_deregistration_state()
{
	so_change_state( m_awaiting_deregistration_state );
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
	ensure_operation_is_on_working_thread();

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
agent_t::so_initiate_agent_definition()
{
	struct working_thread_id_sentinel
		{
			std::thread::id & m_id;

			working_thread_id_sentinel( std::thread::id & id_var )
				:	m_id( id_var )
				{
					m_id = std::this_thread::get_id();
				}
			~working_thread_id_sentinel()
				{
					m_id = std::thread::id();
				}
		}
	sentinel( m_working_thread_id );

	so_define_agent();

	m_was_defined = true;
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

so_environment_t &
agent_t::so_environment()
{
	return m_so_environment_impl->query_public_so_environment();
}

void
agent_t::so_bind_to_dispatcher(
	std::thread::id working_thread_id,
	event_queue_t & queue )
{
	// Cooperation usage counter should be incremented.
	// It will be decremented during final agent event execution.
	agent_coop_t::increment_usage_count( *m_agent_coop );

	m_working_thread_id = working_thread_id;

	m_tmp_event_queue.switch_to_actual_queue(
			queue,
			this,
			&agent_t::demand_handler_on_start );

	m_event_queue.store( &queue );
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
}

void
agent_t::shutdown_agent()
{
	// Since v.5.2.3.4 shutdown is done by three steps:
	//
	// 1. Lock object, set deregistration status and move subscription
	// map to different location.
	//
	// 2. Unlock object and remove all subscription. Subscriptions must be
	// removed on unlocked object to avoid deadlocks on mbox operations (see for
	// example: https://sourceforge.net/p/sobjectizer/bugs/10/).
	//
	// 3. Send the last demand to the agent.

	consumers_map_t subscriptions;
	{
		// Step #1. Must be done on locked object.
		std::lock_guard< std::mutex > lock( m_mutex );

		m_is_coop_deregistered = true;
		m_event_consumers_map.swap( subscriptions );
	}

	// Step #2. Must be done on unlocked object.
	
	// Subscriptions should be destroyed.
	destroy_all_subscriptions( subscriptions );

	m_event_queue.load( std::memory_order_acquire )->push(
			this,
			event_caller_block_ref_t(),
			message_ref_t(),
			&agent_t::demand_handler_on_finish );
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
	const std::type_index & type_index,
	const mbox_ref_t & mbox_ref,
	const state_t & target_state,
	const event_handler_method_t & method )
{
	ensure_operation_is_on_working_thread();

	subscription_key_t subscr_key( type_index, mbox_ref );

	mbox_subscription_management_proxy_t mbox_proxy( mbox_ref );

	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	if( m_is_coop_deregistered )
		return;

	consumers_map_t::iterator it = m_event_consumers_map.find( subscr_key );

	// If subscription is not found then it should be created.
	if( m_event_consumers_map.end() == it )
	{
		create_and_register_event_caller_block(
				type_index,
				mbox_proxy,
				target_state,
				method,
				subscr_key );
	}
	else
	{
		it->second->insert( target_state, std::move(method) );
	}
}

void
agent_t::create_and_register_event_caller_block(
	const std::type_index & type_index,
	mbox_subscription_management_proxy_t & mbox_proxy,
	const state_t & target_state,
	const event_handler_method_t & method,
	const subscription_key_t & subscr_key )
{
	event_caller_block_ref_t caller_block(
			new event_caller_block_t() );
	caller_block->insert( target_state, method );

	mbox_proxy.subscribe_event_handler(
		type_index,
		this,
		caller_block.get() );

	// If we won't add event into event_consumers_map then
	// event must be removed from mbox.
	try
	{
		m_event_consumers_map.insert(
			consumers_map_t::value_type(
				subscr_key,
				caller_block ) );
	}
	catch( ... )
	{
		mbox_proxy.unsubscribe_event_handlers( type_index, this );
		throw;
	}
}

void
agent_t::destroy_all_subscriptions(
	consumers_map_t & subscriptions )
{
	consumers_map_t::iterator it = subscriptions.begin();
	consumers_map_t::iterator it_end = subscriptions.end();

	for(; it != it_end; ++it )
	{
		mbox_subscription_management_proxy_t mbox_proxy( it->first.second );

		mbox_proxy.unsubscribe_event_handlers(
			it->first.first,
			this );
	}
}

void
agent_t::do_drop_subscription(
	const std::type_index & type_index,
	const mbox_ref_t & mbox_ref,
	const state_t & target_state )
{
	ensure_operation_is_on_working_thread();

	subscription_key_t subscr_key( type_index, mbox_ref );

	mbox_subscription_management_proxy_t mbox_proxy( mbox_ref );

	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	consumers_map_t::iterator it = m_event_consumers_map.find( subscr_key );

	if( m_event_consumers_map.end() != it )
	{
		it->second->remove_caller_for_state( target_state );

		if( it->second->empty() )
		{
			mbox_proxy.unsubscribe_event_handlers(
				it->first.first,
				this );

			// There is no more interest in that subscription key.
			m_event_consumers_map.erase( it );
		}
	}
}

void
agent_t::do_drop_subscription_for_all_states(
	const std::type_index & type_index,
	const mbox_ref_t & mbox_ref )
{
	ensure_operation_is_on_working_thread();

	subscription_key_t subscr_key( type_index, mbox_ref );

	mbox_subscription_management_proxy_t mbox_proxy( mbox_ref );

	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	consumers_map_t::iterator it = m_event_consumers_map.find( subscr_key );

	if( m_event_consumers_map.end() != it )
	{
		mbox_proxy.unsubscribe_event_handlers(
			it->first.first,
			this );

		// There is no more interest in that subscription key.
		m_event_consumers_map.erase( it );
	}
}

void
agent_t::push_event(
	const event_caller_block_ref_t & event_caller_block,
	const message_ref_t & message )
{
	m_event_queue.load( std::memory_order_acquire )->push(
			this,
			event_caller_block,
			message,
			&agent_t::demand_handler_on_message );
}

void
agent_t::push_service_request(
	const event_caller_block_ref_t & event_caller_block,
	const message_ref_t & message )
{
	m_event_queue.load( std::memory_order_acquire )->push(
			this,
			event_caller_block,
			message,
			&agent_t::service_request_handler_on_message );
}

void
agent_t::demand_handler_on_start(
	message_ref_t &,
	const event_caller_block_t *,
	agent_t * agent )
{
	agent->so_evt_start();
}

void
agent_t::demand_handler_on_finish(
	message_ref_t &,
	const event_caller_block_t *,
	agent_t * agent )
{
	agent->so_evt_finish();
	// Cooperation should receive notification about agent deregistration.
	agent_coop_t::decrement_usage_count( *(agent->m_agent_coop) );
}

void
agent_t::demand_handler_on_message(
	message_ref_t & msg,
	const event_caller_block_t * event_handler,
	agent_t * agent )
{
	event_handler->call(
			agent->so_current_state(),
			invocation_type_t::event,
			msg );
}

void
agent_t::service_request_handler_on_message(
	message_ref_t & msg,
	const event_caller_block_t * event_handler,
	agent_t * agent )
{
	try
		{
			if( !event_handler->call(
					agent->so_current_state(),
					invocation_type_t::service_request,
					msg ) )
				SO_5_THROW_EXCEPTION(
						so_5::rc_svc_not_handled,
						"service request handler is not found for "
								"the current agent state" );
		}
	catch( ... )
		{
			auto & svc_request =
					*(dynamic_cast< msg_service_request_base_t * >( msg.get() ));
			svc_request.set_exception( std::current_exception() );
		}
}

void
agent_t::ensure_operation_is_on_working_thread() const
{
	if( std::this_thread::get_id() != m_working_thread_id )
		SO_5_THROW_EXCEPTION(
				so_5::rc_operation_enabled_only_on_agent_working_thread,
				"operation is enabled only on agent's working thread" );
}

} /* namespace rt */

} /* namespace so_5 */

