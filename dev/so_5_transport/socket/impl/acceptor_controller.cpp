/*
	SObjectizer 5 Transport.
*/

#include <so_5/h/log_err.hpp>

#include <so_5_transport/socket/impl/h/acceptor_controller.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// socket_acceptor_controller_t
//

socket_acceptor_controller_t::socket_acceptor_controller_t(
	const acceptor_params_t & params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	ACE_Reactor * reactor )
	:
		m_params( params ),
		m_channel_params( channel_params ),
		m_option_setter( std::move( option_setter ) ),
		m_reactor( reactor )
{
}

socket_acceptor_controller_t::~socket_acceptor_controller_t()
{
}

so_5::ret_code_t
socket_acceptor_controller_t::create(
	const so_5::rt::mbox_ref_t & ta_mbox )
{
	m_acceptor.reset(
		new acceptor_t(
			m_channel_params,
			ta_mbox,
			m_option_setter.get() ) );

	if( !m_acceptor.get() )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "Create acceptor failed" ) ) );

		return rc_acceptor_creation_failed;
	}

	if( -1 == m_acceptor->open(
			m_params.local_addr(),
			m_reactor,
			m_params.flags(),
			m_params.use_select(),
			m_params.reuse_addr() ) )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "Acceptor::open() failed" ) ) );

		return rc_acceptor_creation_failed;
	}

	return 0;
}

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

