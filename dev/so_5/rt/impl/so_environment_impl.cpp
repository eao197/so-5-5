/*
	SObjectizer 5.
*/

#include <exception>

#include <so_5/h/exception.hpp>
#include <so_5/h/ret_code.hpp>

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

	timer_thread::timer_id_t timer_id = m_timer_thread->schedule_act(
			std::move(timer_act) );

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

	m_timer_thread->schedule_act( std::move(timer_act) );
}

so_layer_t *
so_environment_impl_t::query_layer(
	const type_wrapper_t & type ) const
{
	return m_layer_core.query_layer( type );
}

void
so_environment_impl_t::add_extra_layer(
	const type_wrapper_t & type,
	const so_layer_ref_t & layer )
{
	m_layer_core.add_extra_layer( type, layer );
}

void
so_environment_impl_t::run(
	so_environment_t & env )
{
	try
	{
		run_layers_and_go_further( env );
	}
	catch( const so_5::exception_t & )
	{
		// Rethrow our exception because it already has all information.
		throw;
	}
	catch( const std::exception & x )
	{
		SO_5_THROW_EXCEPTION(
				rc_environment_error,
				std::string( "some unexpected error during "
						"environment launching: " ) + x.what() );
	}
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

void
so_environment_impl_t::run_layers_and_go_further(
	so_environment_t & env )
{
	do_run_stage(
			"run_layers",
			[this] { m_layer_core.start(); },
			[this] { m_layer_core.finish(); },
			[this, &env] { run_dispatcher_and_go_further( env ); } );
}

void
so_environment_impl_t::run_dispatcher_and_go_further(
	so_environment_t & env )
{
	do_run_stage(
			"run_dispatcher",
			[this] { m_disp_core.start(); },
			[this] { m_disp_core.finish(); },
			[this, &env] { run_timer_and_go_further( env ); } );
}

void
so_environment_impl_t::run_timer_and_go_further(
	so_environment_t & env )
{
	do_run_stage(
			"run_timer",
			[this] { m_timer_thread->start(); },
			[this] { m_timer_thread->finish(); },
			[this, &env] { run_agent_core_and_go_further( env ); } );
}

void
so_environment_impl_t::run_agent_core_and_go_further(
	so_environment_t & env )
{
	do_run_stage(
			"run_agent_core",
			[this] { m_agent_core.start(); },
			[this] { m_agent_core.finish(); },
			[this, &env] { run_user_supplied_init_and_wait_for_stop( env ); } );
}

void
so_environment_impl_t::run_user_supplied_init_and_wait_for_stop(
	so_environment_t & env )
{
	try
	{
		// Initilizing environment.
		env.init();
		m_agent_core.wait_for_start_deregistration();
	}
	catch( const std::exception & ex )
	{
		env.stop();
		m_agent_core.wait_for_start_deregistration();

		throw;
	}
}

void
so_environment_impl_t::do_run_stage(
	const std::string & stage_name,
	std::function< void() > init_fn,
	std::function< void() > deinit_fn,
	std::function< void() > next_stage )
{
	try
	{
		init_fn();
	}
	catch( const std::exception & x )
	{
		SO_5_THROW_EXCEPTION(
				rc_unexpected_error,
				stage_name + ": initialization failed, exception is: '" +
				x.what() + "'" );
	}

	try
	{
		next_stage();
	}
	catch( const std::exception & x )
	{
		try
		{
			deinit_fn();
		}
		catch( const std::exception & nested )
		{
			SO_5_THROW_EXCEPTION(
					rc_unexpected_error,
					stage_name + ": deinitialization failed during "
					"exception handling. Original exception is: '" + x.what() +
					"', deinitialization exception is: '" + nested.what() + "'" );
		}

		throw;
	}

	try
	{
		deinit_fn();
	}
	catch( const std::exception & x )
	{
		SO_5_THROW_EXCEPTION(
				rc_unexpected_error,
				stage_name + ": deinitialization failed, exception is: '" +
				x.what() + "'" );
	}
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

