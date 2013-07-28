/*
	SObjectizer 5 Transport.
*/

#include <algorithm>

#include <so_5/h/log_err.hpp>

#include <so_5/rt/h/mbox.hpp>

#include <so_5_transport/h/a_transport_agent.hpp>

namespace so_5_transport
{

//
// a_transport_agent_t
//

a_transport_agent_t::a_transport_agent_t(
	so_5::rt::so_environment_t & env )
	:
		base_type_t( env ),
		m_failed_state( self_ptr() ),
		m_self_mbox( env.create_local_mbox() )
{
}

a_transport_agent_t::~a_transport_agent_t()
{
}

void
a_transport_agent_t::so_define_agent()
{
	so_subscribe( m_self_mbox )
		.event( &a_transport_agent_t::evt_channel_created );

	so_subscribe( m_self_mbox )
		.event( &a_transport_agent_t::evt_channel_lost );
}

// Для использования std::for_each
void
close_channel( const channel_controller_ref_t & controller )
{
	channel_controller_ref_t ctrl( controller );

	// Игнорируем код возврата.
	ctrl->close();
}

void
a_transport_agent_t::so_evt_finish()
{
	std::for_each(
		m_controllers.begin(),
		m_controllers.end(),
		close_channel );
}

void
a_transport_agent_t::evt_channel_created(
	const so_5::rt::event_data_t< msg_channel_created > & msg )
{
	m_controllers.insert( msg->m_controller );
}

void
a_transport_agent_t::evt_channel_lost(
	const so_5::rt::event_data_t< msg_channel_lost > & msg )
{
	m_controllers.erase( msg->m_controller );

	// Дополнительная обработка.
	on_lost( *msg );
}

void
a_transport_agent_t::on_lost( const msg_channel_lost & )
{
	// Реализация по умолчанию ничего не делает.
}

//
// a_server_transport_agent_t
//

a_server_transport_agent_t::a_server_transport_agent_t(
	so_5::rt::so_environment_t & env,
	acceptor_controller_unique_ptr_t acceptor_controller )
	:
		base_type_t( env ),
		m_acceptor_controller( std::move( acceptor_controller ) )
{
}

a_server_transport_agent_t::~a_server_transport_agent_t()
{
}

void
a_server_transport_agent_t::so_evt_start()
{
	bool succeed_res = true;
	if( 0 != m_acceptor_controller->create(
		m_self_mbox ) )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "a_server_transport_agent: create acceptor failed" ) ) );

		m_acceptor_controller.reset();
		so_change_state( m_failed_state );

		succeed_res = false;
	}

	m_self_mbox->deliver_message(
		std::unique_ptr< msg_create_server_socket_result >(
			new msg_create_server_socket_result( succeed_res ) ) );
}

void
a_server_transport_agent_t::so_evt_finish()
{
	m_acceptor_controller.reset();
	base_type_t::so_evt_finish();
}

//
// client_reconnect_params_t
//

client_reconnect_params_t::client_reconnect_params_t(
	unsigned int on_lost_reconnect_timeout,
	unsigned int on_failed_reconnect_timeout,
	bool do_reconnect )
	:
		m_on_lost_reconnect_timeout( on_lost_reconnect_timeout ),
		m_on_failed_reconnect_timeout( on_failed_reconnect_timeout ),
		m_do_reconnect( do_reconnect )
{
}

client_reconnect_params_t::~client_reconnect_params_t()
{
}

//
// a_client_transport_agent_t
//

a_client_transport_agent_t::a_client_transport_agent_t(
	so_5::rt::so_environment_t & env,
	connector_controller_unique_ptr_t connector_controller,
	const client_reconnect_params_t & client_reconnect_params )
	:
		base_type_t( env ),
		m_connector_controller( std::move( connector_controller ) ),
		m_client_reconnect_params( client_reconnect_params )
{
}

a_client_transport_agent_t::~a_client_transport_agent_t()
{
}

void
a_client_transport_agent_t::so_define_agent()
{
	base_type_t::so_define_agent();

	// Подписываемся на переподключение, если это указано.
	if( m_client_reconnect_params.m_do_reconnect )
		so_subscribe( m_self_mbox )
			.event( &a_client_transport_agent_t::evt_reconnect );
}

void
a_client_transport_agent_t::so_evt_start()
{
	do_reconnect();
}

void
a_client_transport_agent_t::so_evt_finish()
{
	m_connector_controller.reset();
	base_type_t::so_evt_finish();
}


void
a_client_transport_agent_t::evt_reconnect(
	const so_5::rt::event_data_t< msg_reconnect > & )
{
	do_reconnect();
}

void
a_client_transport_agent_t::on_lost( const msg_channel_lost & )
{
	if( m_client_reconnect_params.m_do_reconnect )
		so_environment().single_timer< msg_reconnect >(
			m_self_mbox,
			m_client_reconnect_params.m_on_lost_reconnect_timeout );
}

void
a_client_transport_agent_t::do_reconnect()
{
	if( !try_to_connect() )
	{
		if( m_client_reconnect_params.m_do_reconnect )
			so_environment().single_timer< msg_reconnect >(
				m_self_mbox,
				m_client_reconnect_params.m_on_failed_reconnect_timeout );
		else
		{
			m_connector_controller.reset();
			// Если не указано что надо переподключаться,
			// то переходим в состояние "ошибка".
			so_change_state( m_failed_state );
		}
	}
}


bool
a_client_transport_agent_t::try_to_connect()
{
	if( 0 != m_connector_controller->connect(
		m_self_mbox ) )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "a_client_transport_agent: connect failed" ) ) );

		return false;
	}

	return true;
}


} /* namespace so_5_transport */
