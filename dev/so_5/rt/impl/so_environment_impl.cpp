/*
	SObjectizer 5.
*/

#include <exception>

#include <so_5/h/exception.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/util/h/apply_throwing_strategy.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/timer_thread/ace_timer_queue_adapter/h/pub.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

so_environment_impl_t::so_environment_impl_t(
	const so_environment_params_t & so_environment_params,
	so_environment_t & public_so_environment )
	:
		m_mbox_core( new mbox_core_t(
			so_environment_params.mbox_mutex_pool_size() ) ),
		m_agent_core(
			public_so_environment,
			so_environment_params
				.agent_coop_mutex_pool_size(),
			so_environment_params
				.agent_event_queue_mutex_pool_size(),
			std::move(
				const_cast< so_environment_params_t & >(
					so_environment_params ).m_coop_listener ) ),
		m_disp_core(
			public_so_environment,
			so_environment_params.named_dispatcher_map(),
			std::move(
				const_cast< so_environment_params_t & >(
					so_environment_params ).m_event_exception_logger ),
			std::move(
				const_cast< so_environment_params_t & >(
					so_environment_params ).m_event_exception_handler ) ),
		m_layer_core(
			so_environment_params.so_layers_map(),
			&public_so_environment ),
		m_public_so_environment( public_so_environment ),
		m_timer_thread(
			std::move(
				const_cast< so_environment_params_t & >(
					so_environment_params ).m_timer_thread ) )
{
	if( 0 == m_timer_thread.get() )
	{
		using namespace so_5::timer_thread::ace_timer_queue_adapter;
		m_timer_thread = create_timer_thread();
	}
}

so_environment_impl_t::~so_environment_impl_t()
{
}

so_5::timer_thread::timer_id_ref_t
so_environment_impl_t::schedule_timer(
	const rt::type_wrapper_t & type_wrapper,
	const message_ref_t & msg,
	const mbox_ref_t & mbox,
	unsigned int delay_msec,
	unsigned int period_msec )
{
	timer_thread::timer_act_unique_ptr_t timer_act(
		new timer_thread::timer_act_t(
			type_wrapper,
			mbox,
			msg,
			delay_msec,
			period_msec ) );

	timer_thread::timer_id_t timer_id = m_timer_thread->schedule_act( timer_act );

	return timer_thread::timer_id_ref_t::create(
		*m_timer_thread,
		timer_id );
}

void
so_environment_impl_t::single_timer(
	const type_wrapper_t & type_wrapper,
	const message_ref_t & msg,
	const mbox_ref_t & mbox,
	unsigned int delay_msec )
{
	timer_thread::timer_act_unique_ptr_t timer_act(
		new timer_thread::timer_act_t(
			type_wrapper,
			mbox,
			msg,
			delay_msec,
			0 ) );

	m_timer_thread->schedule_act( timer_act );
}

so_layer_t *
so_environment_impl_t::query_layer(
	const type_wrapper_t & type ) const
{
	return m_layer_core.query_layer( type );
}

ret_code_t
so_environment_impl_t::add_extra_layer(
	const type_wrapper_t & type,
	const so_layer_ref_t & layer,
	throwing_strategy_t throwing_strategy )
{
	return m_layer_core.add_extra_layer(
		type,
		layer,
		throwing_strategy );
}

ret_code_t
so_environment_impl_t::run(
	so_environment_t & env,
	throwing_strategy_t throwing_strategy )
{
	try
	{
		const ret_code_t rc = m_layer_core.start();

		if( rc )
		{
			return so_5::util::apply_throwing_strategy(
				rc,
				throwing_strategy,
				"failed to start layers" );
			return rc;
		}

		// Starting dispatchers...
		m_disp_core.start();

		// Starting timer...
		m_timer_thread->start();

		// Starting an agent utility...
		m_agent_core.start();

//FIXME: those actions could be moved to dedicated method.
		bool init_threw = false;
		std::string init_exception_reason;
		try
		{
			// Initilizing environment.
			env.init();
		}
		catch( const std::exception & ex )
		{
			init_threw = true;
			init_exception_reason = ex.what();
			env.stop();
		}

		// Wait for deregistration signal...
		m_agent_core.wait_for_start_deregistration();

		// All agents should start their shutdown...
		m_agent_core.shutdown();

		// Informs about shutdown...
		m_timer_thread->shutdown();
		m_disp_core.shutdown();

		// Wait for agents...
		m_agent_core.wait();

		// Wait timer...
		m_timer_thread->wait();

		// Wait dispatcher...
		m_disp_core.wait();

		// Shutdown and wait extra layers.
		m_layer_core.shutdown_extra_layers();
		m_layer_core.wait_extra_layers();
		// Shutdown and wait default layers.
		m_layer_core.shutdown_default_layers();
		m_layer_core.wait_default_layers();

		if( init_threw )
		{
			return so_5::util::apply_throwing_strategy(
				rc_environment_error,
				throwing_strategy,
				"init() failed: " + init_exception_reason );
		}
	}
	catch( const exception_t & ex )
	{
		std::cerr << "SO error: " << ex.what() << std::endl;
		return so_5::util::apply_throwing_strategy(
			ex,
			throwing_strategy );
	}
	catch( const std::exception & ex )
	{
		return so_5::util::apply_throwing_strategy(
			rc_environment_error,
			throwing_strategy,
			ex.what() );
	}

	return 0;
}

void
so_environment_impl_t::stop()
{
	// Sends shutdown signal for all agents.
	m_agent_core.start_deregistration();
}

so_environment_t &
so_environment_impl_t::query_public_so_environment()
{
	return m_public_so_environment;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

