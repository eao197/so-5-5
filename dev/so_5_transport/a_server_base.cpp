/*
	SObjectizer 5 Transport.
*/

#include <so_5/rt/h/rt.hpp>

#include <so_5_transport/h/a_server_base.hpp>

namespace so_5_transport
{

//
// a_server_base_t
//

a_server_base_t::a_server_base_t(
	so_5::rt::so_environment_t & env,
	const so_5::rt::mbox_ref_t & notification_mbox )
	:
		base_type_t( env, notification_mbox )
{
}

a_server_base_t::~a_server_base_t()
{
}

void
a_server_base_t::so_define_agent()
{
	base_type_t::so_define_agent();

	for( state_set_t::iterator
		it = m_states_listeninig_channel_events.begin(),
		it_end = m_states_listeninig_channel_events.end();
		it != it_end;
		++it )
	{
		so_subscribe( query_notification_mbox() )
			.in( **it )
			.event( &a_server_base_t::evt_create_server_socket_result );
	}
}

void
a_server_base_t::evt_create_server_socket_result(
	const so_5::rt::event_data_t< msg_create_server_socket_result > &
		msg )
{
	so_handle_create_server_socket_result( *msg );
}

void
a_server_base_t::so_handle_create_server_socket_result(
	const msg_create_server_socket_result & msg )
{
	if( !msg.m_succeed )
	{
		so_environment().deregister_coop(
			so_5::rt::nonempty_name_t( so_coop_name() ) );
	}
}

} /* namespace so_5_transport */
