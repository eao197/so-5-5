/*
	SObjectizer 5 SOP4
*/

#include <oess_2/io/h/subbinstream.hpp>

#include <so_5/h/exception.hpp>
#include <so_5/h/log_err.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/h/mbapi_layer.hpp>

#include <mbapi_4/comm/impl/h/iochannel_data_processor.hpp>
#include <mbapi_4/comm/impl/h/get_layer_impl.hpp>

#include <mbapi_4/comm/h/a_mbapi_outgoing_channel.hpp>

namespace mbapi_4
{

namespace comm
{

a_mbapi_outgoing_channel_t::a_mbapi_outgoing_channel_t(
	so_5::rt::so_environment_t & env,
	const so_5::rt::mbox_ref_t & notification_mbox,
	const handshaking_params_t & handshaking_params )
	:
		base_type_t( env, notification_mbox ),
		m_mbapi_layer(
			impl::get_layer_impl(
				so_environment().query_layer< mbapi_4::mbapi_layer_t >() ) ),
		m_handshaking_params( handshaking_params )
{
}

a_mbapi_outgoing_channel_t::~a_mbapi_outgoing_channel_t()
{
}

void
a_mbapi_outgoing_channel_t::so_evt_start()
{
	so_subscribe( query_notification_mbox() )
		.event( &a_mbapi_outgoing_channel_t::evt_sync );
	so_subscribe( query_notification_mbox() )
		.event( &a_mbapi_outgoing_channel_t::evt_transmit );
}

void
a_mbapi_outgoing_channel_t::so_evt_finish()
{
	if( 0 != m_server_channel.get() )
		m_mbapi_layer.delete_channel( m_server_channel->channel_uid() );

	m_server_channel.reset();
}

void
a_mbapi_outgoing_channel_t::evt_transmit(
	const so_5::rt::event_data_t< transmit_info_t > & msg )
{
	try
	{
		if( msg.get() &&
			0 != m_server_channel.get() )
			m_server_channel->process_outgoing(
				const_cast< transmit_info_t & >( *msg ) );
	}
	catch( const std::exception & ex )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT( "error sendimg message, "
				"error: %s; "
				"from: %s; "
				"to: %s; "
				"stage: (%s, %s); "
				"type: %s" ),
			ex.what(),
			msg->m_from.name().c_str(),
			msg->m_to.name().c_str(),
			msg->m_current_stage.name().c_str(),
			msg->m_current_stage.endpoint_name().c_str(),
			msg->m_oess_id.str().c_str() ));
	}
}

void
a_mbapi_outgoing_channel_t::evt_sync(
	const so_5::rt::event_data_t< tables_sync_message_t > & msg )
{
	if( 0 != m_server_channel.get() )
	{
		// Выполняем синхронизацию.
		m_server_channel->sync_tables( m_mbapi_layer );

		// Проверяем активность.
		m_server_channel->check_activity();
	}
}


void
a_mbapi_outgoing_channel_t::so_handle_client_connected(
	const so_5_transport::msg_channel_created & msg )
{
	try
	{
		if( 0 == m_server_channel.get() )
		{
			m_server_channel.reset(
				new impl::server_channel_data_processor_t(
					m_handshaking_params,
					query_notification_mbox(),
					msg.m_controller,
					msg.m_io ) );

			m_server_channel->initiate_handshake(
				m_mbapi_layer.node_uid() );

			msg.m_controller->enforce_input_detection();

			m_sync_timer_id =
				so_environment().schedule_timer< tables_sync_message_t >(
					query_notification_mbox(),
					100,
					1 * 1000 );
		}
		else
		{
			// Такого быть не должно
			ACE_ERROR(( LM_ERROR,
				SO_5_LOG_FMT( "error connecting new client: double connect event" ) ));
		}
	}
	catch( const std::exception & ex )
	{
		msg.m_controller->close();
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT( "error connecting new client: %d; error: %s" ),
			msg.m_channel_id, ex.what() ));
	}
}

void
a_mbapi_outgoing_channel_t::so_handle_client_failed(
	const so_5_transport::msg_channel_failed & msg )
{
}

void
a_mbapi_outgoing_channel_t::so_handle_client_disconnected(
	const so_5_transport::msg_channel_lost & msg )
{
	if( 0 != m_server_channel.get() )
		m_mbapi_layer.delete_channel( m_server_channel->channel_uid() );

	m_server_channel.reset();
	m_sync_timer_id.release();
}

void
a_mbapi_outgoing_channel_t::so_handle_incoming_package(
	const so_5_transport::msg_incoming_package & msg )
{
	try
	{
		if( 0 != m_server_channel.get() )
		{
			so_5_transport::input_trx_unique_ptr_t itrx = msg.begin_input_trx();
			m_server_channel->process_incoming(
				m_mbapi_layer,
				*itrx );
		}
	}
	catch( const std::exception & ex )
	{
		m_server_channel->close();
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT(
				"a_mbapi_outgoing_channel_t::so_handle_incoming_package "
				"error processing client: %d; error: %s" ),
			msg.m_channel_id, ex.what() ));
	}
}

} /* namespace comm */

} /* namespace mbapi_4 */
