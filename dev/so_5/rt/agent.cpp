/*
	SObjectizer 5.
*/

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/state_listener_controller.hpp>

#include <sstream>

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

	subscription_map_t subscriptions;
	{
		// Step #1. Must be done on locked object.
		std::lock_guard< std::mutex > lock( m_mutex );

		m_is_coop_deregistered = true;
		m_subscriptions.swap( subscriptions );
	}

	// Step #2. Must be done on unlocked object.
	
	// Subscriptions should be destroyed.
	destroy_all_subscriptions( subscriptions );

	m_event_queue.load( std::memory_order_acquire )->push(
			execution_demand_t(
					this,
					0,
					typeid(void),
					message_ref_t(),
					&agent_t::demand_handler_on_finish ) );
}

namespace
{
	template< class S >
	bool is_known_mbox_msg_pair(
		S & s,
		typename S::iterator it )
	{
		if( it != s.begin() )
		{
			typename S::iterator prev = it;
			++prev;
			if( it->first.is_same_mbox_msg_pair( prev->first ) )
				return true;
		}

		typename S::iterator next = it;
		++next;
		if( next != s.end() )
			return it->first.is_same_mbox_msg_pair( next->first );

		return false;
	}

	std::string
	make_subscription_description(
		const mbox_ref_t & mbox_ref,
		std::type_index msg_type,
		const state_t & state )
	{
		std::ostringstream s;
		s << "(mbox:'" << mbox_ref->query_name()
			<< "', msg_type:'" << msg_type.name() << "', state:'"
			<< state.query_name() << "')";

		return s.str();
	}

} /* namespace anonymous */

void
agent_t::create_event_subscription(
	const mbox_ref_t & mbox_ref,
	std::type_index type_index,
	const state_t & target_state,
	const event_handler_method_t & method )
{
	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	ensure_operation_is_on_working_thread();

	if( m_is_coop_deregistered )
		return;

	auto insertion_result = m_subscriptions.emplace(
			subscription_key_t( mbox_ref->id(), type_index, target_state ),
			subscription_data_t( mbox_ref, method ) );

	if( !insertion_result.second )
		SO_5_THROW_EXCEPTION(
			rc_evt_handler_already_provided,
			"agent is already subscribed to message, " +
			make_subscription_description( mbox_ref, type_index, target_state ) );

	auto mbox_msg_known = is_known_mbox_msg_pair(
			m_subscriptions, insertion_result.first );
	if( !mbox_msg_known )
	{
		// Mbox must create subscription.
		try
		{
			mbox_ref->subscribe_event_handler( type_index, this );
		}
		catch( ... )
		{
			// Rollback agent's subscription.
			m_subscriptions.erase( insertion_result.first );
			throw;
		}
	}
}

void
agent_t::destroy_all_subscriptions(
	subscription_map_t & subscriptions )
{
	auto it = subscriptions.begin();
	auto it_end = subscriptions.end();

	for(; it != it_end; ++it )
	{
		it->second.m_mbox->unsubscribe_event_handlers(
			it->first.m_msg_type,
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

	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	auto it = m_subscriptions.find(
			subscription_key_t( mbox_ref->id(), type_index, target_state ) );

	if( m_subscriptions.end() != it )
	{
		bool mbox_msg_known = is_known_mbox_msg_pair( m_subscriptions, it );

		m_subscriptions.erase( it );

		if( !mbox_msg_known )
		{
			mbox_ref->unsubscribe_event_handlers(
				type_index,
				this );
		}
	}
}

void
agent_t::do_drop_subscription_for_all_states(
	const std::type_index & type_index,
	const mbox_ref_t & mbox_ref )
{
	// Since v.5.4.0 there is no need for locking agent's mutex
	// because this operation can be performed only on agent's
	// working thread.

	ensure_operation_is_on_working_thread();

	subscription_key_t key( mbox_ref->id(), type_index );

	auto it = m_subscriptions.lower_bound( key );
	if( it != m_subscriptions.end() )
	{
		while( key.is_same_mbox_msg_pair( it->first ) )
			m_subscriptions.erase( it++ );

		mbox_ref->unsubscribe_event_handlers( type_index, this );
	}
}

void
agent_t::push_event(
	mbox_id_t mbox_id,
	std::type_index msg_type,
	const message_ref_t & message )
{
	m_event_queue.load( std::memory_order_acquire )->push(
			execution_demand_t(
				this,
				mbox_id,
				msg_type,
				message,
				&agent_t::demand_handler_on_message ) );
}

void
agent_t::push_service_request(
	mbox_id_t mbox_id,
	std::type_index msg_type,
	const message_ref_t & message )
{
	m_event_queue.load( std::memory_order_acquire )->push(
			execution_demand_t(
					this,
					mbox_id,
					msg_type,
					message,
					&agent_t::service_request_handler_on_message ) );
}

void
agent_t::demand_handler_on_start( execution_demand_t & d )
{
	d.m_receiver->so_evt_start();
}

void
agent_t::demand_handler_on_finish( execution_demand_t & d )
{
	d.m_receiver->so_evt_finish();
	// Cooperation should receive notification about agent deregistration.
	agent_coop_t::decrement_usage_count( *(d.m_receiver->m_agent_coop) );
}

void
agent_t::demand_handler_on_message( execution_demand_t & d )
{
	auto it = d.m_receiver->m_subscriptions.find(
			subscription_key_t(
					d.m_mbox_id,
					d.m_msg_type, 
					d.m_receiver->so_current_state() ) );
	if( it != d.m_receiver->m_subscriptions.end() )
		it->second.m_method( invocation_type_t::event, d.m_message_ref );
}

void
agent_t::service_request_handler_on_message( execution_demand_t & d )
{
	try
		{
			auto it = d.m_receiver->m_subscriptions.find(
					subscription_key_t(
							d.m_mbox_id,
							d.m_msg_type, 
							d.m_receiver->so_current_state() ) );
			if( it != d.m_receiver->m_subscriptions.end() )
				it->second.m_method(
						invocation_type_t::service_request,
						d.m_message_ref );
			else
				SO_5_THROW_EXCEPTION(
						so_5::rc_svc_not_handled,
						"service request handler is not found for "
								"the current agent state" );
		}
	catch( ... )
		{
			auto & svc_request =
					*(dynamic_cast< msg_service_request_base_t * >(
							d.m_message_ref.get() ));
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

