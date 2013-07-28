/*
	SObjectizer 5 Transport.
*/

#include <so_5/h/log_err.hpp>

#include <so_5_transport/socket/impl/h/connector_controller.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// socket_connector_controller_t
//

socket_connector_controller_t::socket_connector_controller_t(
	const connector_params_t & connector_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	ACE_Reactor * reactor )
	:
		m_connector_params( connector_params ),
		m_channel_params( channel_params ),
		m_option_setter( std::move( option_setter ) ),
		m_connector( reactor, ACE_NONBLOCK )
{
}

so_5::ret_code_t
socket_connector_controller_t::connect(
	const so_5::rt::mbox_ref_t & ta_mbox )
{
	ACE_TRACE( "so_5_transport::socket::impl::"
		"socket_connector_controller_t::connect" );

	// Для того, чтобы дать возможность установить нестандартные
	// опции для SOCK_Stream приходится создавать svc_handler_t
	// вручную.
	svc_handler_t * handler = new svc_handler_t(
		m_connector.reactor(),
		1,
		ta_mbox,
		m_channel_params );

	if( m_option_setter.get() )
	{
		so_5::ret_code_t r = m_option_setter->setup( handler->peer() );
		if( !r )
			return r;
	}

	const int connect_result = m_connector.connect(
		handler,
		m_connector_params.remote_addr(),
		m_connector_params.synch_options(),
		m_connector_params.local_addr(),
		m_connector_params.reuse_addr(),
		m_connector_params.flags() );

	if( -1 == connect_result )
	{
		const int reactor_used =
			m_connector_params.synch_options()[
				ACE_Synch_Options::USE_REACTOR ];
		// Если соединение устанавливается в асинхронном режиме
		// с использованием реактора, то код ошибки EWOULDBLOCK
		// не должен рассматриваться как ошибка.
		if( !( reactor_used && EWOULDBLOCK == ACE_OS::last_error() ) )
		{
			// Соединение установить не удалось.
			const int error = ACE_OS::last_error();
			char remote_addr[ 128 ] = { 0 };
			m_connector_params.remote_addr().addr_to_string(
					remote_addr,
					sizeof( remote_addr ) - 1 );

			ACE_ERROR( (
				LM_ERROR,
				SO_5_LOG_FMT( "connect(): failed; errno: %d; remote_addr: %s" ),
					error,
					remote_addr ) );

			return rc_connect_failed;
		}
	}

	return 0;
}

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

