/*
	SObjectizer 5.
*/

#include <so_5/rt/impl/h/so_environment_impl.hpp>

#include <so_5/rt/h/so_environment.hpp>

namespace so_5
{

namespace rt
{

//
// so_environment_params_t
//

so_environment_params_t::so_environment_params_t()
	:
		m_mbox_mutex_pool_size( 128 ),
		m_agent_coop_mutex_pool_size( 32 ),
		m_agent_event_queue_mutex_pool_size( 128 ),
		m_event_exception_logger( create_std_event_exception_logger() ),
		m_event_exception_handler( create_std_event_exception_handler() )
{
}

so_environment_params_t::~so_environment_params_t()
{
}

so_environment_params_t &
so_environment_params_t::mbox_mutex_pool_size(
	unsigned int mutex_pool_size )
{
	m_mbox_mutex_pool_size = mutex_pool_size;
	return * this;
}

so_environment_params_t &
so_environment_params_t::agent_coop_mutex_pool_size(
	unsigned int mutex_pool_size )
{
	m_agent_coop_mutex_pool_size = mutex_pool_size;
	return * this;
}

so_environment_params_t &
so_environment_params_t::agent_event_queue_mutex_pool_size(
	unsigned int mutex_pool_size )
{
	m_agent_event_queue_mutex_pool_size = mutex_pool_size;
	return * this;
}

so_environment_params_t &
so_environment_params_t::add_named_dispatcher(
	const nonempty_name_t & name,
	dispatcher_unique_ptr_t dispatcher )
{
	m_named_dispatcher_map[ name.query_name() ] =
		dispatcher_ref_t( dispatcher.release() );
	return *this;
}

so_environment_params_t &
so_environment_params_t::timer_thread(
	so_5::timer_thread::timer_thread_unique_ptr_t timer_thread )
{
	m_timer_thread = std::move( timer_thread );
	return  *this;
}

so_environment_params_t &
so_environment_params_t::coop_listener(
	coop_listener_unique_ptr_t coop_listener )
{
	m_coop_listener = std::move( coop_listener );
	return *this;
}

so_environment_params_t &
so_environment_params_t::event_exception_logger(
	event_exception_logger_unique_ptr_t logger )
{
	if( nullptr != logger.get() )
		m_event_exception_logger = std::move( logger );

	return *this;
}

so_environment_params_t &
so_environment_params_t::event_exception_handler(
	event_exception_handler_unique_ptr_t handler )
{
	if( nullptr != handler.get() )
		m_event_exception_handler = std::move( handler );

	return *this;
}

void
so_environment_params_t::add_layer(
	const type_wrapper_t & type,
	so_layer_unique_ptr_t layer_ptr )
{
	m_so_layers[ type ] = so_layer_ref_t( layer_ptr.release() );
}

unsigned int
so_environment_params_t::mbox_mutex_pool_size() const
{
	return m_mbox_mutex_pool_size;
}

unsigned int
so_environment_params_t::agent_coop_mutex_pool_size() const
{
	return m_agent_coop_mutex_pool_size;
}

unsigned int
so_environment_params_t::agent_event_queue_mutex_pool_size() const
{
	return m_agent_event_queue_mutex_pool_size;
}

const named_dispatcher_map_t &
so_environment_params_t::named_dispatcher_map() const
{
	return m_named_dispatcher_map;
}

const so_layer_map_t &
so_environment_params_t::so_layers_map() const
{
	return m_so_layers;
}

//
// so_environment_t
//

so_environment_t &
so_environment_t::self_ref()
{
	return *this;
}


so_environment_t::so_environment_t(
	so_environment_params_t && so_environment_params )
	:
		m_so_environment_impl(
			new impl::so_environment_impl_t(
				std::move(so_environment_params),
				self_ref() ) )
{
}

so_environment_t::~so_environment_t()
{
	if( m_so_environment_impl )
		delete m_so_environment_impl;
}

mbox_ref_t
so_environment_t::create_local_mbox( )
{
	return m_so_environment_impl->create_local_mbox();
}

mbox_ref_t
so_environment_t::create_local_mbox(
	const nonempty_name_t & nonempty_name )
{
	return m_so_environment_impl->create_local_mbox( nonempty_name );
}

mbox_ref_t
so_environment_t::create_local_mbox(
	std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr )
{
	return m_so_environment_impl->create_local_mbox(
		std::move( lock_ptr ) );
}

mbox_ref_t
so_environment_t::create_local_mbox(
	const nonempty_name_t & nonempty_name,
	std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr )
{
	return m_so_environment_impl->create_local_mbox(
		nonempty_name,
		std::move( lock_ptr ) );
}

dispatcher_t &
so_environment_t::query_default_dispatcher()
{
	return m_so_environment_impl->query_default_dispatcher();
}

dispatcher_ref_t
so_environment_t::query_named_dispatcher(
	const std::string & disp_name )
{
	return m_so_environment_impl
		->query_named_dispatcher( disp_name );
}

void
so_environment_t::install_exception_logger(
	event_exception_logger_unique_ptr_t logger )
{
	m_so_environment_impl->install_exception_logger( std::move( logger ) );
}

void
so_environment_t::install_exception_handler(
	event_exception_handler_unique_ptr_t handler )
{
	m_so_environment_impl->install_exception_handler( std::move( handler ) );
}

agent_coop_unique_ptr_t
so_environment_t::create_coop(
	const nonempty_name_t & name )
{
	return create_coop(
		name,
		create_default_disp_binder() );
}

agent_coop_unique_ptr_t
so_environment_t::create_coop(
	const nonempty_name_t & name,
	disp_binder_unique_ptr_t disp_binder )
{
	return agent_coop_t::create_coop(
		name,
		std::move(disp_binder),
		self_ref() );
}

void
so_environment_t::register_coop(
	agent_coop_unique_ptr_t agent_coop )
{
	m_so_environment_impl->register_coop( std::move( agent_coop ) );
}

void
so_environment_t::deregister_coop(
	const nonempty_name_t & name )
{
	m_so_environment_impl->deregister_coop( name );
}

so_5::timer_thread::timer_id_ref_t
so_environment_t::schedule_timer(
	const rt::type_wrapper_t & type_wrapper,
	const message_ref_t & msg,
	const mbox_ref_t & mbox,
	unsigned int delay_msec,
	unsigned int period_msec )
{
	return m_so_environment_impl->schedule_timer(
		type_wrapper, msg, mbox, delay_msec, period_msec );
}

void
so_environment_t::single_timer(
	const type_wrapper_t & type_wrapper,
	const message_ref_t & msg,
	const mbox_ref_t & mbox,
	unsigned int delay_msec )
{
	m_so_environment_impl->single_timer(
		type_wrapper, msg, mbox, delay_msec );
}

so_layer_t *
so_environment_t::query_layer(
	const type_wrapper_t & type ) const
{
	return m_so_environment_impl->query_layer( type );
}

void
so_environment_t::add_extra_layer(
	const type_wrapper_t & type,
	const so_layer_ref_t & layer )
{
	m_so_environment_impl->add_extra_layer( type, layer );
}

void
so_environment_t::run()
{
	m_so_environment_impl->run( *this );
}

void
so_environment_t::stop()
{
	m_so_environment_impl->stop();
}

impl::so_environment_impl_t &
so_environment_t::so_environment_impl()
{
	return *m_so_environment_impl;
}

} /* namespace rt */

} /* namespace so_5 */
