/*
	SObjectizer 5 Transport.
*/

#include <so_5/h/exception.hpp>

#include <so_5_transport/ace/h/reactors.hpp>

#include <so_5_transport/h/ret_code.hpp>

#include <so_5_transport/socket/h/pub.hpp>

#include <so_5_transport/socket/impl/h/acceptor_controller.hpp>
#include <so_5_transport/socket/impl/h/connector_controller.hpp>

namespace so_5_transport
{

namespace socket
{

//
// modify_old_format_ip
//

SO_5_TRANSPORT_EXPORT_FUNC_SPEC( std::string )
modify_old_format_ip( const std::string & ip )
{
	if( !ip.empty() && ':' == ip[ 0 ] )
		return "localhost" + ip;

	return ip;
}

//
// option_setter_t
//

option_setter_t::~option_setter_t()
{}

//
// connector_params_t
//

connector_params_t::connector_params_t(
	const ACE_INET_Addr & remote_addr )
	:
		m_synch_options( ACE_Synch_Options::defaults ),
		m_reuse_addr( 0 ),
		m_flags( O_RDWR ),
		m_remote_addr( remote_addr )
{
}

connector_params_t::~connector_params_t()
{
}

const ACE_Synch_Options &
connector_params_t::synch_options() const
{
	return m_synch_options;
}

connector_params_t &
connector_params_t::set_synch_options(
	const ACE_Synch_Options & value )
{
	m_synch_options = value;
	return *this;
}

int
connector_params_t::reuse_addr() const
{
	return m_reuse_addr;
}

connector_params_t &
connector_params_t::set_reuse_addr(
	int value )
{
	m_reuse_addr = value;
	return *this;
}

int
connector_params_t::flags() const
{
	return m_flags;
}

connector_params_t &
connector_params_t::set_flags(
	int value )
{
	m_flags = value;
	return *this;
}

const ACE_INET_Addr &
connector_params_t::remote_addr() const
{
	return m_remote_addr;
}

connector_params_t &
connector_params_t::set_remote_addr(
	const ACE_INET_Addr & value )
{
	m_remote_addr = value;
	return *this;
}

const ACE_INET_Addr &
connector_params_t::local_addr() const
{
	return m_local_addr;
}

connector_params_t &
connector_params_t::set_local_addr(
	const ACE_INET_Addr & value )
{
	m_local_addr = value;
	return *this;
}

//
// create_connector_params
//

SO_5_TRANSPORT_EXPORT_FUNC_SPEC( connector_params_t )
create_connector_params( const std::string & remote_addr )
{
	return connector_params_t(
		ACE_INET_Addr( modify_old_format_ip( remote_addr ).c_str() ) );
}

//
// acceptor_params_t
//

acceptor_params_t::acceptor_params_t(
	const ACE_INET_Addr & local_addr )
	:
		m_local_addr( local_addr ),
		m_reuse_addr( 1 ),
		m_flags( ACE_NONBLOCK ),
		m_use_select( 1 )
{
}

acceptor_params_t::~acceptor_params_t()
{
}

const ACE_INET_Addr &
acceptor_params_t::local_addr() const
{
	return m_local_addr;
}

acceptor_params_t &
acceptor_params_t::set_local_addr(
	const ACE_INET_Addr & value )
{
	m_local_addr = value;
	return *this;
}

int
acceptor_params_t::reuse_addr() const
{
	return m_reuse_addr;
}

acceptor_params_t &
acceptor_params_t::set_reuse_addr(
	int value )
{
	m_reuse_addr = value;
	return *this;
}

int
acceptor_params_t::flags() const
{
	return m_flags;
}

acceptor_params_t &
acceptor_params_t::set_flags(
	int value )
{
	m_flags = value;
	return *this;
}

int
acceptor_params_t::use_select() const
{
	return m_use_select;
}

acceptor_params_t &
acceptor_params_t::set_use_select(
	int value )
{
	m_use_select = value;
	return *this;
}


//
// create_acceptor_params
//

SO_5_TRANSPORT_EXPORT_FUNC_SPEC( acceptor_params_t )
create_acceptor_params( const std::string & local_addr )
{
	return acceptor_params_t(
		ACE_INET_Addr( modify_old_format_ip( local_addr ).c_str() ) );
}

//
// acceptor_controller_creator_t
//

acceptor_controller_creator_t::acceptor_controller_creator_t(
	so_5::rt::so_environment_t & env )
	:
		m_reactor_layer( env.query_layer< reactor_layer_t >() )
{
}

acceptor_controller_creator_t::~acceptor_controller_creator_t()
{
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const std::string & ip_address,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create( create_acceptor_params( ip_address ), throwing_strategy );
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const acceptor_params_t & acceptor_params,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create( acceptor_params, channel_params_t(), throwing_strategy );
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const acceptor_params_t & acceptor_params,
	const channel_params_t & channel_params,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create(
		acceptor_params,
		channel_params,
		option_setter_unique_ptr_t(),
		throwing_strategy );
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const acceptor_params_t & acceptor_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( m_reactor_layer )
		return create(
			acceptor_params,
			channel_params,
			std::move( option_setter ),
			m_reactor_layer->query_default_reactor(),
			throwing_strategy );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor layer is not set",
				rc_reactor_layer_is_not_set );
	}

	return acceptor_controller_unique_ptr_t();
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const acceptor_params_t & acceptor_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	const std::string & reactor_name,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( m_reactor_layer )
		return create(
			acceptor_params,
			channel_params,
			std::move( option_setter ),
			m_reactor_layer->query_named_reactor( reactor_name ),
			throwing_strategy );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor layer is not set",
				rc_reactor_layer_is_not_set );
	}

	return acceptor_controller_unique_ptr_t();
}

acceptor_controller_unique_ptr_t
acceptor_controller_creator_t::create(
	const acceptor_params_t & acceptor_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	ACE_Reactor * reactor,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( reactor )
		return acceptor_controller_unique_ptr_t(
			new impl::socket_acceptor_controller_t(
				acceptor_params,
				channel_params,
				std::move( option_setter ),
				reactor ) );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor not found",
				rc_reactor_not_found );
	}

	return acceptor_controller_unique_ptr_t();
}

//
// create_connector_controller
//

connector_controller_creator_t::connector_controller_creator_t(
	so_5::rt::so_environment_t & env )
	:
		m_reactor_layer( env.query_layer< reactor_layer_t >() )
{
}

connector_controller_creator_t::~connector_controller_creator_t()
{
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const std::string & ip_address,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create( create_connector_params( ip_address ), throwing_strategy );
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const connector_params_t & connector_params,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create( connector_params, channel_params_t(), throwing_strategy );
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const connector_params_t & connector_params,
	const channel_params_t & channel_params,
	so_5::throwing_strategy_t throwing_strategy )
{
	return create(
		connector_params,
		channel_params,
		option_setter_unique_ptr_t(),
		throwing_strategy );
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const connector_params_t & connector_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( m_reactor_layer )
		return create(
			connector_params,
			channel_params,
			std::move( option_setter ),
			m_reactor_layer->query_default_reactor(),
			throwing_strategy );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor layer is not set",
				rc_reactor_layer_is_not_set );
	}

	return connector_controller_unique_ptr_t();
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const connector_params_t & connector_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	const std::string & reactor_name,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( m_reactor_layer )
		return create(
			connector_params,
			channel_params,
			std::move( option_setter ),
			m_reactor_layer->query_named_reactor( reactor_name ),
			throwing_strategy );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor layer is not set",
				rc_reactor_layer_is_not_set );
	}

	return connector_controller_unique_ptr_t();
}

connector_controller_unique_ptr_t
connector_controller_creator_t::create(
	const connector_params_t & connector_params,
	const channel_params_t & channel_params,
	option_setter_unique_ptr_t option_setter,
	ACE_Reactor * reactor,
	so_5::throwing_strategy_t throwing_strategy )
{
	if( reactor )
		return connector_controller_unique_ptr_t(
			new impl::socket_connector_controller_t(
				connector_params,
				channel_params,
				std::move( option_setter ),
				reactor ) );
	else
	{
		if( so_5::THROW_ON_ERROR == throwing_strategy )
			throw so_5::exception_t(
				"reactor not found",
				rc_reactor_not_found );
	}

	return connector_controller_unique_ptr_t();
}

} /* namespace socket */

} /* namespace so_5_transport */

