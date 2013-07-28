/*
	Пример, демонстрирующий работу с серверным raw-соединением.

	Производится попытка подключения к указанному адресу,
	после чего в соединение отсылаются все необязательные
	аргументы командной строки. После этого осуществляется
	переход в режим чтения и отображения данных.
*/

#include <iostream>
#include <vector>
#include <string>

#include <ace/OS_main.h>

// Базовые заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Диспетчер активных агентов.
#include <so_5/disp/active_obj/h/pub.hpp>


// Описание агента, который обслуживает клиентский
// канал и средств для создания каналов.
#include <so_5_transport/h/reactor_layer.hpp>
#include <so_5_transport/h/channel_io.hpp>
#include <so_5_transport/h/channel_controller.hpp>
#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/a_transport_agent.hpp>
#include <so_5_transport/h/a_client_base.hpp>
#include <so_5_transport/socket/h/pub.hpp>

// Средства для создания tcp соединений.
#include <so_5_transport/socket/h/pub.hpp>

//
// Класс агента, который будет работать с серверным соединением.
//
class	a_main_t
	:
		public so_5_transport::a_client_base_t
{
	typedef so_5_transport::a_client_base_t base_type_t;

	public :
		a_main_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & notification_mbox,
			const std::vector< std::string > & rest_of_argv )
			:
				base_type_t( env, notification_mbox ),
				m_rest_of_argv( rest_of_argv )
		{}

		virtual ~a_main_t()
		{}

		// Реакция на подключение клиента.
		virtual void
		so_handle_client_connected(
			const so_5_transport::msg_channel_created & msg );

		// Реакция на неудавшееся подключение клиента.
		virtual void
		so_handle_client_failed(
			const so_5_transport::msg_channel_failed & msg );

		// Реакция на отключение клиента.
		virtual void
		so_handle_client_disconnected(
			const so_5_transport::msg_channel_lost & msg );

		// Реакция на поступление данных в канал.
		virtual void
		so_handle_incoming_package(
			const so_5_transport::msg_incoming_package & msg );

	private :
		const std::vector< std::string > m_rest_of_argv;
};

void
a_main_t::so_handle_client_connected(
	const so_5_transport::msg_channel_created & msg )
{
	std::cout << "\na_main_t::so_handle_client_connected\n"
		"channel id: " << msg.m_channel_id << "\n"
		"local address:  "
			<< msg.m_controller->local_address_as_string() << "\n"
		"remote address: "
			<< msg.m_controller->remote_address_as_string() << std::endl;

	// Активируем чтение из сокета.
	msg.m_controller->enforce_input_detection();

	so_5_transport::output_trx_unique_ptr_t out_trx = msg.m_io.begin_output_trx();

	for( size_t i = 0; i < m_rest_of_argv.size(); ++i )
	{
		out_trx->ostream().write(
			m_rest_of_argv[ i ].data(),
			m_rest_of_argv[ i ].size() );

		char newlines [] = "\r\n\r\n";
		out_trx->ostream().write( newlines, 4 );
	}

	out_trx->commit();
}

void
a_main_t::so_handle_client_failed(
	const so_5_transport::msg_channel_failed & msg )
{
	std::cout << "\na_main_t::so_handle_client_failed\n"
		"reason: "<< msg.m_reason << std::endl;

	so_environment().stop();
}

void
a_main_t::so_handle_client_disconnected(
	const so_5_transport::msg_channel_lost & msg )
{
	std::cout << "\na_main_t::so_handle_client_disconnected\n"
		"channel id: " << msg.m_channel_id << "\n"
		"reason: "<< msg.m_reason << std::endl;
}

void
a_main_t::so_handle_incoming_package(
	const so_5_transport::msg_incoming_package & msg )
{
	so_5_transport::input_trx_unique_ptr_t in_trx = msg.begin_input_trx();

	const size_t stream_size = in_trx->stream_size();
	if( stream_size > 0 )
	{
		std::cout << "\na_main_t::so_handle_incoming_package\n"
			"channel id: " << msg.m_channel_id << "\n"
			"available data size: " << stream_size << "\n"
			"data:\n";

		std::vector< char > data( stream_size );
		in_trx->istream().read( &data[0], stream_size );

		std::cout.write( &data[0], stream_size );

		std::cout << std::endl;
	}
	in_trx->commit();
}

struct client_data_t
{
	std::string m_server_addr;
	std::vector< std::string > m_rest_of_argv;

	void
	init( so_5::rt::so_environment_t & env )
	{
		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( "test_client_application" ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );

		so_5_transport::socket::connector_controller_creator_t
			connector_creator( env );

		using so_5_transport::a_client_transport_agent_t;
		std::unique_ptr< a_client_transport_agent_t > ta(
			new a_client_transport_agent_t(
				env,
				connector_creator.create( m_server_addr ) ) );

		so_5::rt::agent_ref_t client(
			new a_main_t(
				env,
				ta->query_notificator_mbox(),
				m_rest_of_argv ) );

		coop->add_agent( client );
		coop->add_agent( so_5::rt::agent_ref_t( ta.release() ) );

		// Регистрируем кооперацию
		so_5::ret_code_t rc = env.register_coop( std::move( coop ) );

		// Если не удается зарегистрировать кооперацию
		// напишем сообщение об ошибке.
		if( 0 != rc )
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
		else
		{

			std::cout << "Type \"quit\" to quit." << std::endl;
			std::string cmd;
			do
			{
				std::cin >> cmd;
			} while( cmd != "quit" );
		}

		env.stop();
	}
};

int
main( int argc, char ** argv )
{
	if( 2 <= argc )
	{
		client_data_t client_data;
		client_data.m_server_addr = argv[ 1 ];

		for( size_t i = 2; i < argc; ++i )
			client_data.m_rest_of_argv.push_back( argv[ i ] );

		return so_5::api::run_so_environment_on_object(
			client_data,
			&client_data_t::init,
			so_5::rt::so_environment_params_t()
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "active_obj" ),
					so_5::disp::active_obj::create_disp() )
				.add_layer(
					std::unique_ptr< so_5_transport::reactor_layer_t >(
						new so_5_transport::reactor_layer_t ) ) );
	}
	else
		std::cerr << "sample.so_5_transport.raw_channel.tcp_cln <port>" << std::endl;

	return 0;
}
