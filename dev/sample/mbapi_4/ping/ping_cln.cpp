/*
	Пример, демонстрирующий работу mbapi_4.

	Создается клиентски mbapi канал.
	Создается агент, который заводит себе сертификат конечной точки,
	и начинает слать ping-сообщения на заданный mbox.
	Имена конечных точек задаются в коммандной строке.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <time.h>

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
#include <mbapi_4/h/endpoint_bind.hpp>
#include <mbapi_4/h/message.hpp>
#include <mbapi_4/comm/h/a_mbapi_outgoing_channel.hpp>

// Сообщений ping/pong.
#include <sample/mbapi_4/ping/h/messages.hpp>


namespace sample_mbapi_4
{

namespace /* ananymous */
{

std::string
now_to_str()
{
	std::ostringstream sout;
	time_t t = time( 0 );
	sout << asctime( localtime( &t ) );

	return sout.str();
}

} /* ananymous namespace */


//
// Класс агента, который выполняет отправку сообщений.
//

class a_main_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public :
		a_main_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::endpoint_t & self_endpoint,
			const mbapi_4::endpoint_t & target_endpoint )
			:
				base_type_t( env ),
				m_self_endpoint( self_endpoint ),
				m_target_endpoint( target_endpoint ),
				m_num_counter( 0 ),
				m_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~a_main_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

		void
		evt_ping(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< ping_t > > & msg )
		{
			std::cout << "ping #" << msg->msg().m_num << "\n"
				<< msg->msg().m_timestamp
				<< "from: " << msg->from().name() << "\n"
				<< "to: " << msg->to().name() << "\n\n";

			// Отсылаем ответ.
			std::unique_ptr< pong_t > pong( new pong_t );

			pong->m_timestamp = now_to_str();
			pong->m_num = msg->msg().m_num;

			m_self_endpoint_bind->send(
				msg->from(),
				std::move( pong ) );
		}

		void
		evt_pong(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< pong_t > > & msg )
		{
			std::cout << "pong #" << msg->msg().m_num << "\n"
				<< msg->msg().m_timestamp
				<< "from: " << msg->from().name() << "\n"
				<< "to: " << msg->to().name() << "\n\n";
		}

		struct tick
			:
				public so_5::rt::message_t
		{};

		void
		evt_send(
			const so_5::rt::event_data_t< tick > & )
		{
			std::unique_ptr< ping_t > ping( new ping_t );

			ping->m_timestamp = now_to_str();
			ping->m_num = ++m_num_counter;

			m_self_endpoint_bind->send(
				m_target_endpoint,
				std::move( ping ) );
		}

	private:
		mbapi_4::endpoint_bind_unique_ptr_t
			m_self_endpoint_bind;

		const mbapi_4::endpoint_t m_self_endpoint;
		const mbapi_4::endpoint_t m_target_endpoint;

		unsigned int m_num_counter;

		//! Таймер для события отправки ping-ов.
		so_5::timer_thread::timer_id_ref_t m_tick_timer_id;

		so_5::rt::mbox_ref_t m_mbox;
};

void
a_main_t::so_define_agent()
{
	m_self_endpoint_bind = so_environment()
		.query_layer< mbapi_4::mbapi_layer_t >()
			->create_endpoint_bind( m_self_endpoint, *this );

	so_subscribe( m_mbox )
		.event( &a_main_t::evt_send );

	m_self_endpoint_bind->subscribe_event(
		so_default_state(),
		&a_main_t::evt_ping );

	m_self_endpoint_bind->subscribe_event(
		so_default_state(),
		&a_main_t::evt_pong );
}

void
a_main_t::so_evt_start()
{
	m_tick_timer_id =
		so_environment().schedule_timer< a_main_t::tick >(
			m_mbox,
			1 * 1000,
			5 * 1000 );
}

struct client_data_t
{
	std::string m_server_addr;
	std::string m_self_endpoint;
	std::string m_target_endpoint;

	void
	register_channel_coop(
		so_5::rt::so_environment_t & env )
	{
		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( "mbapi_client_channel" ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );

		so_5_transport::socket::connector_controller_creator_t
			connector_creator( env );

		using so_5_transport::a_client_transport_agent_t;
		std::unique_ptr< a_client_transport_agent_t > ta(
			new a_client_transport_agent_t(
				env,
				connector_creator.create( m_server_addr ) ) );

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
	add_ping_messages_oess_repository(
		so_5::rt::so_environment_t & env )
	{
		env.query_layer< mbapi_4::mbapi_layer_t >()
			->add_repository( ping_t::get_repository() );
	}

	void
	init( so_5::rt::so_environment_t & env )
	{
		register_channel_coop( env );
		add_ping_messages_oess_repository( env );

		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( "main_coop" ) );
		so_5::rt::agent_ref_t a_main(
			new a_main_t(
				env,
				mbapi_4::endpoint_t( m_self_endpoint ),
				mbapi_4::endpoint_t( m_target_endpoint ) ) );

		coop->add_agent( a_main );

		// Регистрируем кооперацию
		const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );

		// Если не удается зарегистрировать кооперацию
		// напишем сообщение об ошибке.
		if( 0 != rc )
		{
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
			throw std::runtime_error( "channel coop not registered" );
		}
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

} /* namespace sample_mbapi_4 */


int
main( int argc, char ** argv )
{
	try
	{
		if( 4 == argc )
		{
			sample_mbapi_4::client_data_t client_data;
			client_data.m_server_addr = argv[ 1 ];
			client_data.m_self_endpoint = argv[ 2 ];
			client_data.m_target_endpoint = argv[ 3 ];

			return so_5::api::run_so_environment_on_object(
				client_data,
				&sample_mbapi_4::client_data_t::init,
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
			std::cerr << "sample.mbapi_4.ping.cln "
				"<ip:port> <self-endpoint> <target-endpoint>" << std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
