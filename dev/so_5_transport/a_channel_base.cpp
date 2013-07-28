/*
	SObjectizer 5 Transport.
*/

#include <so_5/rt/h/rt.hpp>

#include <so_5_transport/h/a_channel_base.hpp>

namespace so_5_transport
{

//
// a_channel_base_t
//

a_channel_base_t::a_channel_base_t(
	so_5::rt::so_environment_t & env,
	const so_5::rt::mbox_ref_t & notification_mbox )
	:
		base_type_t( env ),
		m_notification_mbox( notification_mbox )
{
	so_add_state_for_channel_events( so_default_state() );
}

a_channel_base_t::~a_channel_base_t()
{
}

void
a_channel_base_t::so_define_agent()
{
	for( state_set_t::iterator
		it = m_states_listeninig_channel_events.begin(),
		it_end = m_states_listeninig_channel_events.end();
		it != it_end;
		++it )
	{
		so_subscribe( query_notification_mbox() )
			.in( **it )
			.event( &a_channel_base_t::evt_channel_created );
		so_subscribe( query_notification_mbox() )
			.in( **it )
			.event( &a_channel_base_t::evt_channel_failed );
		so_subscribe( query_notification_mbox() )
			.in( **it )
			.event( &a_channel_base_t::evt_channel_lost );
		so_subscribe( query_notification_mbox() )
			.in( **it )
			.event( &a_channel_base_t::evt_incoming_package );
	}
}


void
a_channel_base_t::evt_channel_created(
	const so_5::rt::event_data_t< msg_channel_created > & msg )
{
	so_handle_client_connected( *msg );
}

void
a_channel_base_t::evt_channel_failed(
	const so_5::rt::event_data_t< msg_channel_failed > & msg )
{
	so_handle_client_failed( *msg );
}

void
a_channel_base_t::evt_channel_lost(
	const so_5::rt::event_data_t< msg_channel_lost > & msg )
{
	so_handle_client_disconnected( *msg );
}

void
a_channel_base_t::evt_incoming_package(
	const so_5::rt::event_data_t<
		so_5_transport::msg_incoming_package > & msg )
{
	so_handle_incoming_package( *msg );
}

void
a_channel_base_t::so_add_state_for_channel_events(
	const so_5::rt::state_t & state )
{
	if( !so_was_defined() )
		m_states_listeninig_channel_events.insert( &state );
}

} /* namespace so_5_transport */
