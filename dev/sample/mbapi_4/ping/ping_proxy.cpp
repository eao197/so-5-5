/*
	Пример, демонстрирующий работу mbapi_4.

	Прокси процес с mbapi слоем.
	Может создавать одновременно несколько клиентскиих серверных каналов.
	Агентов сам не создает, а служит промежуточным звеном для соединения
	mbapi-узлов.
*/

#include <iostream>
#include <string>

#include <cpp_util_2/h/lexcast.hpp>

// Базовые заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Диспетчер активных агентов.
#include <so_5/disp/active_obj/h/pub.hpp>

// Описание агента, который обслуживает клиентский
// канал, и средств для создания каналов.
#include <so_5_transport/h/reactor_layer.hpp>
#include <so_5_transport/h/a_transport_agent.hpp>
#include <so_5_transport/h/a_client_base.hpp>

// Средства для создания tcp соединений.
#include <so_5_transport/socket/h/pub.hpp>

// Заголовочные файлы mbapi_4.
#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/h/mbapi_layer.hpp>
#include <mbapi_4/h/message.hpp>
#include <mbapi_4/comm/h/a_mbapi_incoming_channel.hpp>
#include <mbapi_4/comm/h/a_mbapi_outgoing_channel.hpp>

namespace sample_mbapi_4
{

typedef std::vector< std::string > addr_list_t;

struct proxy_data_t
{
	addr_list_t m_server_addrs;
	addr_list_t m_client_addrs;

	void
	register_server_channel_coop(
		so_5::rt::so_environment_t & env,
		const std::string & coop_name,
		const std::string & addr )
	{
		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( coop_name ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );

		so_5_transport::socket::acceptor_controller_creator_t
			acceptor_creator( env );

		using so_5_transport::a_server_transport_agent_t;
		std::unique_ptr< a_server_transport_agent_t > ta(
			new a_server_transport_agent_t(
				env,
				acceptor_creator.create( addr ) ) );

		so_5::rt::mbox_ref_t comm_mbox = env.create_local_mbox();

		so_5::rt::agent_ref_t
			a_mbapi_incoming_channel(
				new mbapi_4::comm::a_mbapi_incoming_channel_t(
					env,
					ta->query_notificator_mbox(),
					mbapi_4::handshaking_params_t() ) );

		coop->add_agent( a_mbapi_incoming_channel );
		coop->add_agent( so_5::rt::agent_ref_t( ta.release() ) );

		// Регистрируем кооперацию
		const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );
		if( 0 != rc )
		{
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
			throw std::runtime_error( "channel coop not registered" );
		}
	}


	void
	register_client_channel_coop(
		so_5::rt::so_environment_t & env,
		const std::string & coop_name,
		const std::string & addr )
	{
		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( coop_name ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );

		so_5_transport::socket::connector_controller_creator_t
			connector_creator( env );

		using so_5_transport::a_client_transport_agent_t;
		std::unique_ptr< a_client_transport_agent_t > ta(
			new a_client_transport_agent_t(
				env,
				connector_creator.create( addr ) ) );

		so_5::rt::mbox_ref_t comm_mbox = env.create_local_mbox();

		so_5::rt::agent_ref_t
			a_mbapi_outgoing_channel(
				new mbapi_4::comm::a_mbapi_outgoing_channel_t(
					env,
					ta->query_notificator_mbox(),
					mbapi_4::handshaking_params_t() ) );
		coop->add_agent( a_mbapi_outgoing_channel );
		coop->add_agent( so_5::rt::agent_ref_t( ta.release() ) );

		// Регистрируем кооперацию
		const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );
		if( 0 != rc )
		{
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
			throw std::runtime_error( "channel coop not registered" );
		}
	}

	void
	init( so_5::rt::so_environment_t & env )
	{
		for(
			size_t i = 0, n = m_server_addrs.size();
			i < n;
			++i )
		{
			register_server_channel_coop(
				env,
				"server_channel_coop_" + cpp_util_2::slexcast( i ),
				m_server_addrs[ i ] );
		}

		for(
			size_t i = 0, n = m_client_addrs.size();
			i < n;
			++i )
		{
			register_client_channel_coop(
				env,
				"client_channel_coop_" + cpp_util_2::slexcast( i ),
				m_client_addrs[ i ] );
		}


		std::cout << "Proxy started.\n"
			"Type \"quit\" to quit." << std::endl;
		std::string cmd;
		do
		{
			std::cin >> cmd;
		} while( cmd != "quit" );

		env.stop();
	}
};

} /* namespace sample_mbapi_4 */


const std::string g_server_arg = "-s";
const std::string g_client_arg = "-c";

int
main( int argc, char ** argv )
{
	try
	{
		sample_mbapi_4::proxy_data_t proxy_data;
		size_t option_index = 1;
		while( option_index + 1 < argc )
		{
			if( argv[ option_index ] == g_server_arg )
				proxy_data.m_server_addrs.push_back( argv[ option_index + 1 ] );
			else if( argv[ option_index ] == g_client_arg )
				proxy_data.m_client_addrs.push_back( argv[ option_index + 1 ] );
			else
				break;
			option_index += 2;
		}

		if( option_index == argc )
		{
			return so_5::api::run_so_environment_on_object(
				proxy_data,
				&sample_mbapi_4::proxy_data_t::init,
				so_5::rt::so_environment_params_t()
					.add_named_dispatcher(
						so_5::rt::nonempty_name_t( "active_obj" ),
						so_5::disp::active_obj::create_disp() )
					.add_layer(
						std::unique_ptr< so_5_transport::reactor_layer_t >(
							new so_5_transport::reactor_layer_t ) )
					.add_layer(
						std::unique_ptr< mbapi_4::mbapi_layer_t >(
							new mbapi_4::mbapi_layer_t ) ) );
		}
		else
			std::cerr << "sample.mbapi_4.ping.proxy [OPTIONS]\n"
				"OPTIONS:\n"
				"\t-s <ip:port>\t server channel param\n"
				"\t-c <ip:port>\t client channel param\n";
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
