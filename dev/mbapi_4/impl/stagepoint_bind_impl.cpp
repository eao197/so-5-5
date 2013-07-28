/*
	MBAPI 4.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/util/h/mutex_pool.hpp>
#include <so_5/util/h/apply_throwing_strategy.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>
#include <mbapi_4/impl/h/iosession.hpp>
#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>
#include <mbapi_4/impl/h/stagepoint_bind_impl.hpp>

namespace mbapi_4
{

namespace impl
{
//
// subscribed_types_table_t
//


//
// stagepoint_bind_impl_t
//

stagepoint_bind_impl_t::stagepoint_bind_impl_t(
	const stagepoint_t & stagepoint,
	so_5::rt::agent_t & agent,
	mbapi_layer_impl_t & mbapi_layer )
	:
		m_stagepoint( stagepoint ),
		m_agent( agent ),
		m_mbapi_layer( mbapi_layer ),
		m_lock( m_mbapi_layer.allocate_mutex() ),
		m_mbox( m_mbapi_layer.so_environment().create_local_mbox() )
{
}

stagepoint_bind_impl_t::~stagepoint_bind_impl_t()
{
	m_mbapi_layer.deallocate_mutex( m_lock );
	m_mbapi_layer.remove_bind( m_stagepoint );
}

so_5::ret_code_t
stagepoint_bind_impl_t::subscribe_event_impl(
	const oess_id_wrapper_t & oess_id,
	event_subscriber_t & event_subscriber,
	std::unique_ptr< message_integrator_t > message_integrator,
	so_5::throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	if( m_subscribed_types_table.binary_message_subscribed() )
	{
		// Осуществиить подписку на заданный тип нельзя!
		return so_5::util::apply_throwing_strategy(
			rc_unable_to_subscribe_on_binary_message,
			throwing_strategy,
			"unable to subscribe on typed message "
			"because binary subscription exists" );
	}

	subscribed_types_table_t::subscribed_types_map_t::iterator it =
		m_subscribed_types_table.m_subscribed_types_map.find( oess_id );

	if( m_subscribed_types_table.m_subscribed_types_map.end() != it )
	{
		return try_subscribe_event(
			it->second,
			event_subscriber,
			throwing_strategy );
	}

	const so_5::ret_code_t rc =
		event_subscriber.subscribe( m_agent, m_mbox );

	if( 0 != rc )
	{
		return so_5::util::apply_throwing_strategy(
			rc,
			throwing_strategy,
			"unable to subscribe" );
	}

	subscription_info_t subscription_info;
	subscription_info.add_state( event_subscriber.state() );
	subscription_info.m_message_integrator.reset(
		message_integrator.release() );
	m_subscribed_types_table.m_subscribed_types_map[ oess_id ] =
		subscription_info;

	return 0;
}

so_5::ret_code_t
stagepoint_bind_impl_t::unsubscribe_event_impl(
	const oess_id_wrapper_t & oess_id,
	event_subscriber_t & event_subscriber,
	std::unique_ptr< message_integrator_t > message_integrator,
	so_5::throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	subscribed_types_table_t::subscribed_types_map_t::iterator it =
		m_subscribed_types_table.m_subscribed_types_map.find( oess_id );

	if( m_subscribed_types_table.m_subscribed_types_map.end() != it )
	{
		subscription_info_t & subscription_info = it->second;

		if( !subscription_info.remove_state( event_subscriber.state() ) )
		{
			return so_5::util::apply_throwing_strategy(
				rc_event_not_subscribed,
				throwing_strategy,
				"agent is not subscribed on this event in selected state" );
		}

		// Если подписок на этот тип сообщений больше нет,
		// то надо удалять subscription_info.
		if( subscription_info.empty() )
			m_subscribed_types_table.m_subscribed_types_map.erase( it );
	}

	const so_5::ret_code_t rc =
		event_subscriber.unsubscribe( m_agent, m_mbox );

	if( 0 != rc )
	{
		return so_5::util::apply_throwing_strategy(
			rc,
			throwing_strategy,
			"unable to unsubscribe" );
	}

	return 0;
}

so_5::ret_code_t
stagepoint_bind_impl_t::bin_msg_subscribe_event_impl(
	event_subscriber_t & event_subscriber,
	so_5::throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	if( m_subscribed_types_table.has_subscribed_types() )
	{
		// Осуществиить подписку на заданный тип нельзя!
		return so_5::util::apply_throwing_strategy(
			rc_unable_to_subscribe_on_binary_message,
			throwing_strategy,
			"unable to subscribe on binary message "
			"because typed subscription exists" );
	}

	return try_subscribe_event(
		m_subscribed_types_table.m_binary_message_subscription_info,
		event_subscriber,
		throwing_strategy );
}

so_5::ret_code_t
stagepoint_bind_impl_t::bin_msg_unsubscribe_event_impl(
	event_subscriber_t & event_subscriber,
	so_5::throwing_strategy_t throwing_strategy )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_lock );
	if( !m_subscribed_types_table
		.m_binary_message_subscription_info
			.remove_state( event_subscriber.state() ) )
	{
		return so_5::util::apply_throwing_strategy(
			rc_event_not_subscribed,
			throwing_strategy,
			"agent is not subscribed on this event in selected state" );
	}

	const so_5::ret_code_t rc =
		event_subscriber.unsubscribe( m_agent, m_mbox );

	if( 0 != rc )
	{
		return so_5::util::apply_throwing_strategy(
			rc,
			throwing_strategy,
			"unable to unsubscribe" );
	}

	return 0;
}

void
stagepoint_bind_impl_t::send_impl(
	const endpoint_t & to,
	const oess_id_wrapper_t & oess_id,
	std::unique_ptr< oess_2::stdsn::serializable_t > msg )
{
	m_mbapi_layer.transmit_message(
		comm::local_transmit_info_unique_ptr_t(
			new comm::local_transmit_info_t(
				endpoint_t( m_stagepoint.endpoint_name() ),
				to,
				m_stagepoint,
				oess_id,
				std::move( msg ) ) ) );
}

void
stagepoint_bind_impl_t::send_binary_message_impl(
	const endpoint_t & to,
	const oess_id_wrapper_t & oess_id,
	const std::string & binary_message )
{
	comm::transmit_info_unique_ptr_t transmit_info(
		new comm::transmit_info_t(
			endpoint_t( m_stagepoint.endpoint_name() ),
			to,
			m_stagepoint,
			oess_id ) );

	transmit_info->m_payload = binary_message;

	m_mbapi_layer.transmit_banary_message( std::move( transmit_info ) );
}

bool
stagepoint_bind_impl_t::try_to_accept_message(
	oess_2::stdsn::ient_t & reader,
	const comm::transmit_info_t & transmit_info ) const
{
	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );

	if( m_subscribed_types_table.has_subscribed_types() )
	{
		subscribed_types_table_t::subscribed_types_map_t::const_iterator it =
			m_subscribed_types_table
				.m_subscribed_types_map
					.find( transmit_info.m_oess_id );

		if( m_subscribed_types_table.m_subscribed_types_map.end() != it )
		{
			it->second.m_message_integrator->read_and_send(
				m_mbapi_layer.mbapi_layer(),
				transmit_info.m_from,
				transmit_info.m_to,
				transmit_info.m_current_stage,
				reader,
				m_mbox );

			return true;
		}
	}
	else if( m_subscribed_types_table.binary_message_subscribed() )
	{
		std::unique_ptr< binary_message_t > bin_msg(
			new binary_message_t(
				m_mbapi_layer.mbapi_layer(),
				transmit_info.m_from,
				transmit_info.m_to,
				transmit_info.m_current_stage,
				transmit_info.m_oess_id,
				transmit_info.m_payload ) );

		m_mbox->deliver_message( bin_msg );

		return true;
	}

	return false;
}

bool
stagepoint_bind_impl_t::try_to_accept_message(
	comm::local_transmit_info_t & local_transmit_info ) const
{
	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_lock );


	if( m_subscribed_types_table.has_subscribed_types() )
	{
		subscribed_types_table_t::subscribed_types_map_t::const_iterator it =
			m_subscribed_types_table.m_subscribed_types_map.find( local_transmit_info.m_oess_id );

		if( m_subscribed_types_table.m_subscribed_types_map.end() != it )
		{
			it->second.m_message_integrator->cast_and_send(
				m_mbapi_layer.mbapi_layer(),
				local_transmit_info.m_from,
				local_transmit_info.m_to,
				local_transmit_info.m_current_stage,
				std::move( local_transmit_info.m_message ),
				m_mbox );

			return true;
		}
	}
	else if( m_subscribed_types_table.binary_message_subscribed() )
	{
		std::unique_ptr< binary_message_t > bin_msg(
			new binary_message_t(
				m_mbapi_layer.mbapi_layer(),
				local_transmit_info.m_from,
				local_transmit_info.m_to,
				local_transmit_info.m_current_stage,
				local_transmit_info.m_oess_id,
				"" ) );

		oess_2::io::obstring_t output_stream( bin_msg->m_bin_message );
		osession_t osession( output_stream );
		osession.oent() << *local_transmit_info.m_message;

		m_mbox->deliver_message( bin_msg );

		return true;
	}

	return false;
}

so_5::ret_code_t
stagepoint_bind_impl_t::try_subscribe_event(
	subscription_info_t & subscription_info,
	event_subscriber_t & event_subscriber,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( subscription_info.has_state( event_subscriber.state() ) )
		return so_5::util::apply_throwing_strategy(
			rc_duplicating_event_subscription,
			throwing_strategy,
			"duplicating subscription on event for the same agent state" );

	const so_5::ret_code_t rc =
		event_subscriber.subscribe( m_agent, m_mbox );

	if( 0 != rc )
	{
		return so_5::util::apply_throwing_strategy(
			rc,
			throwing_strategy,
			"unable to subscribe" );
	}

	subscription_info.add_state( event_subscriber.state() );

	return 0;
}


} /* namespace impl */

} /* namespace mbapi_4 */
