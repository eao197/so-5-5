/*
	Пример, демонстрирующий работу с серверным raw-соединением.

	Создается серверное соединение и отображаются события,
	которые происходят с этим соединением.
*/

#include <iostream>

#include <ace/OS_main.h>

// Базовые заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Диспетчер активных агентов.
#include <so_5/disp/active_obj/h/pub.hpp>

// Описание агента, который обслуживает серверный
// канал и средств для создания каналов.
#include <so_5_transport/h/reactor_layer.hpp>
#include <so_5_transport/h/channel_io.hpp>
#include <so_5_transport/h/channel_controller.hpp>
#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/a_transport_agent.hpp>
#include <so_5_transport/h/a_server_base.hpp>
#include <so_5_transport/socket/h/pub.hpp>

// Средства для создания tcp соединений.
#include <so_5_transport/socket/h/pub.hpp>

//
// Класс агента, который будет обрабатывать данные.
//

class a_main_t
	:
		public so_5_transport::a_server_base_t
{
		typedef so_5_transport::a_server_base_t base_type_t;

	public:
		a_main_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & notification_mbox )
			:
				base_type_t( env, notification_mbox )
		{}

		virtual ~a_main_t()
		{}

		// Обработка статуса создания сокета.
		virtual void
		so_handle_create_server_socket_result(
			const so_5_transport::msg_create_server_socket_result & msg );

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

	private:
		so_5::rt::mbox_ref_t m_notification_mbox;
};

void
a_main_t::so_handle_create_server_socket_result(
	const so_5_transport::msg_create_server_socket_result & msg )
{
	std::cout << "\na_main_t::so_handle_create_server_socket_result\n";
	if( msg.m_succeed )
		std::cout << "server socket successfuly created.";
	else
	{
		std::cout << "server socket not created.";

		// Останавливаем выполнение среды SO.
		so_environment().stop();
	}

	std::cout << std::endl;
}

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
}

void
a_main_t::so_handle_client_failed(
	const so_5_transport::msg_channel_failed & msg )
{
	std::cout << "\na_main_t::so_handle_client_failed\n"
		"reason: "<< msg.m_reason << std::endl;
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

		so_5_transport::output_trx_unique_ptr_t out_trx =
			msg.begin_output_trx();

		out_trx->ostream().write( &data[0], stream_size );
		out_trx->commit();

		std::cout << std::endl;
	}
	in_trx->commit();
}

void
init( so_5::rt::so_environment_t & env, const std::string & server_addr )
{
	// Создаем кооперацию
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "test_server_application" ),
		so_5::disp::active_obj::create_disp_binder(
			"active_obj" ) );

	so_5_transport::socket::acceptor_controller_creator_t
		acceptor_creator( env );

	using so_5_transport::a_server_transport_agent_t;
	std::unique_ptr< a_server_transport_agent_t > ta(
		new a_server_transport_agent_t(
			env,
			acceptor_creator.create( server_addr ) ) );

	so_5::rt::agent_ref_t serv(
		new a_main_t( env, ta->query_notificator_mbox() ) );

	coop->add_agent( serv );
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

int
main( int argc, char ** argv )
{
	if( 2 == argc )
	{
		std::string server_addr( argv[ 1 ] );

		return so_5::api::run_so_environment_with_parameter(
			&init,
			server_addr,
			so_5::rt::so_environment_params_t()
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "active_obj" ),
					so_5::disp::active_obj::create_disp() )
				.add_layer(
					std::unique_ptr< so_5_transport::reactor_layer_t >(
						new so_5_transport::reactor_layer_t ) ) );
	}
	else
		std::cerr << "sample.so_5_transport.raw_channel.tcp_srv <port>" << std::endl;

	return 0;
}
