/*
	Пример, демонстрирующий работу mbapi_4.

	Прокси процес с mbapi слоем.
	Может создавать одновременно несколько клиентскиих серверных каналов.
	Агентов сам не создает, а служит промежуточным звеном для соединения
	mbapi-узлов.
*/

#include <iostream>
#include <utility>
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
#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/h/mbapi_layer.hpp>
#include <mbapi_4/h/endpoint_bind.hpp>
#include <mbapi_4/h/message.hpp>
#include <mbapi_4/comm/h/a_mbapi_incoming_channel.hpp>
#include <mbapi_4/comm/h/a_mbapi_outgoing_channel.hpp>

// Сообщений ping/pong.
#include <sample/mbapi_4/stages/h/messages.hpp>

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

std::ostream &
operator << ( std::ostream & o, const std::vector< std::string > & v )
{
	o << "{";
	for(
		size_t i = 0, n = v.size();
		i < n;
		++i )
	{
		if( i ) o << ", ";

		o << v[i];
	}

	return o << "}";
}

//
// Класс агента, который обрабатывает сообщения конечной точки.
//

class a_endpoint_handler_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public :
		a_endpoint_handler_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::endpoint_stage_chain_t & self_endpoint,
			const mbapi_4::endpoint_t & target_endpoint )
			:
				base_type_t( env ),
				m_self_endpoint( self_endpoint ),
				m_target_endpoint( target_endpoint ),
				m_num_counter( 0 ),
				m_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~a_endpoint_handler_t()
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
				<< "to: " << msg->to().name() << "\n"
				<< "currentstage: "
					<< msg->current_stage().name() << "@"
					<< msg->current_stage().endpoint_name() << "\n"
				<< "stages passed: " << msg->msg().m_stages_passed << "\n\n";

			// Отсылаем ответ.
			std::unique_ptr< pong_t > pong( new pong_t );

			pong->m_timestamp = now_to_str();
			pong->m_num = msg->msg().m_num;
			pong->m_stages_passed.push_back(
				m_self_endpoint_bind->endpoint().name() + "@" +
				m_self_endpoint_bind->endpoint().name() );

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
				<< "to: " << msg->to().name() << "\n"
				<< "currentstage: "
					<< msg->current_stage().name() << "@"
					<< msg->current_stage().endpoint_name() << "\n"
				<< "stages passed: " << msg->msg().m_stages_passed << "\n\n";
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
			ping->m_stages_passed.push_back(
				m_self_endpoint_bind->endpoint().name() + "@" +
				m_self_endpoint_bind->endpoint().name() );

			m_self_endpoint_bind->send(
				m_target_endpoint,
				std::move( ping ) );
		}

	private:
		mbapi_4::endpoint_bind_unique_ptr_t
			m_self_endpoint_bind;

		const mbapi_4::endpoint_stage_chain_t m_self_endpoint;
		const mbapi_4::endpoint_t m_target_endpoint;

		unsigned int m_num_counter;

		//! Таймер для события отправки ping-ов.
		so_5::timer_thread::timer_id_ref_t m_tick_timer_id;

		so_5::rt::mbox_ref_t m_mbox;
};

void
a_endpoint_handler_t::so_define_agent()
{
	m_self_endpoint_bind = so_environment()
		.query_layer< mbapi_4::mbapi_layer_t >()
			->create_endpoint_bind( m_self_endpoint, *this );

	so_subscribe( m_mbox )
		.event( &a_endpoint_handler_t::evt_send );

	m_self_endpoint_bind->subscribe_event(
		so_default_state(),
		&a_endpoint_handler_t::evt_ping );

	m_self_endpoint_bind->subscribe_event(
		so_default_state(),
		&a_endpoint_handler_t::evt_pong );
}

void
a_endpoint_handler_t::so_evt_start()
{
	m_tick_timer_id =
		so_environment().schedule_timer< a_endpoint_handler_t::tick >(
			m_mbox,
			1 * 1000,
			5 * 1000 );
}

//
// Класс агента, который обрабатывает сообщения точки-стадии.
//

class a_stagepoint_handler_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public :
		a_stagepoint_handler_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::stagepoint_t & self_stagepoint )
			:
				base_type_t( env ),
				m_self_stagepoint( self_stagepoint )
		{}

		virtual ~a_stagepoint_handler_t()
		{}

		virtual void
		so_define_agent();

		void
		evt_ping(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< ping_t > > & msg )
		{
			std::cout << "\tping #" << msg->msg().m_num << "\n"
				<< "\t" << msg->msg().m_timestamp
				<< "\tfrom: " << msg->from().name() << "\n"
				<< "\tto: " << msg->to().name() << "\n"
				<< "\tcurrentstage: "
					<< msg->current_stage().name() << "@"
					<< msg->current_stage().endpoint_name() << "\n"
				<< "\tstages passed: " << msg->msg().m_stages_passed << "\n\n";

			// Создадим копию сообщения, добавим к ней наименования стадии,
			// и отправим дальше копию.
			std::unique_ptr< ping_t > modified_ping( new ping_t( msg->msg() ) );

			modified_ping->m_stages_passed.push_back(
				m_self_stagepoint_bind->stagepoint().name() + "@" +
				m_self_stagepoint_bind->stagepoint().endpoint_name() );

			msg->move_next( std::move( modified_ping ) );
		}

	private:
		mbapi_4::stagepoint_bind_unique_ptr_t
			m_self_stagepoint_bind;

		const mbapi_4::stagepoint_t m_self_stagepoint;
};

void
a_stagepoint_handler_t::so_define_agent()
{
	m_self_stagepoint_bind = so_environment()
		.query_layer< mbapi_4::mbapi_layer_t >()
			->create_stagepoint_bind( m_self_stagepoint, *this );

	m_self_stagepoint_bind->subscribe_event(
		so_default_state(),
		&a_stagepoint_handler_t::evt_ping );
}

typedef std::vector< std::pair< std::string, bool >  > addr_list_t;

typedef std::vector< std::string > stagepoints_list_t;

struct endpoints_data_t
{
	std::string m_ep_desc;
	std::string m_target_endpoint;
};

typedef std::vector< endpoints_data_t > endpoints_list_t;

struct app_data_t
{
	addr_list_t m_server_addrs;
	addr_list_t m_client_addrs;

	stagepoints_list_t m_stagepoints;
	endpoints_list_t m_endpoints;

	void
	register_server_channel_coop(
		so_5::rt::so_environment_t & env,
		const std::string & coop_name,
		const std::string & addr,
		bool compression )
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
					mbapi_4::handshaking_params_t( compression ) ) );

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
		const std::string & addr,
		bool compression )
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
					mbapi_4::handshaking_params_t( compression ) ) );
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
	register_stagepoint_agent_coop(
		so_5::rt::so_environment_t & env,
		const std::string & coop_name,
		const std::string & stage_desc )
	{
			// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( coop_name ) );
		so_5::rt::agent_ref_t a_stagepoint_handler(
			new a_stagepoint_handler_t(
				env,
				mbapi_4::stagepoint_t::create_from_string( stage_desc ) ) );

		coop->add_agent( a_stagepoint_handler );

		// Регистрируем кооперацию
		const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );

		// Если не удается зарегистрировать кооперацию
		// напишем сообщение об ошибке.
		if( 0 != rc )
		{
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
			throw std::runtime_error( "stagepoint coop not registered" );
		}
	}

	void
	register_endpoint_agent_coop(
		so_5::rt::so_environment_t & env,
		const std::string & coop_name,
		const endpoints_data_t & endpoint_data )
	{
		// Создаем кооперацию
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( coop_name ) );
		so_5::rt::agent_ref_t a_endpoint_handler(
			new a_endpoint_handler_t(
				env,
				mbapi_4::endpoint_stage_chain_t::create_from_string(
					endpoint_data.m_ep_desc ),
				mbapi_4::endpoint_t( endpoint_data.m_target_endpoint ) ) );

		coop->add_agent( a_endpoint_handler );

		// Регистрируем кооперацию
		const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );

		// Если не удается зарегистрировать кооперацию
		// напишем сообщение об ошибке.
		if( 0 != rc )
		{
			std::cerr << "reg coop ret_code = " << rc << "\n\n";
			throw std::runtime_error( "endpoint coop not registered" );
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
		add_ping_messages_oess_repository( env );

		for(
			size_t i = 0, n = m_server_addrs.size();
			i < n;
			++i )
		{
			register_server_channel_coop(
				env,
				"server_channel_coop_" + cpp_util_2::slexcast( i ),
				m_server_addrs[ i ].first,
				m_server_addrs[ i ].second );
		}

		for(
			size_t i = 0, n = m_client_addrs.size();
			i < n;
			++i )
		{
			register_client_channel_coop(
				env,
				"client_channel_coop_" + cpp_util_2::slexcast( i ),
				m_client_addrs[ i ].first,
				m_client_addrs[ i ].second );
		}

		for(
			size_t i = 0, n = m_stagepoints.size();
			i < n;
			++i )
		{
			register_stagepoint_agent_coop(
				env,
				"stagepoint_coop_" + cpp_util_2::slexcast( i ),
				m_stagepoints[ i ] );
		}

		for(
			size_t i = 0, n = m_endpoints.size();
			i < n;
			++i )
		{
			register_endpoint_agent_coop(
				env,
				"endpoint_coop_" + cpp_util_2::slexcast( i ),
				m_endpoints[ i ] );
		}

		std::cout << "Type \"quit\" to quit." << std::endl;
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

const std::string g_server_with_compression_arg = "-zs";
const std::string g_client_with_compression_arg = "-zc";

const std::string g_stagepoint_arg = "--stagepoint";
const std::string g_enpoint_arg = "--endpoint";

void
print_usage( std::ostream & o )
{
	o << "sample.mbapi_4.stages OPTIONS\n"
		"OPTIONS:\n"
		"\t" << g_server_arg <<  " <ip:port>\t server channel param\n"
		"\t" << g_client_arg <<  " <ip:port>\t client channel param\n"
		"\t" << g_server_with_compression_arg << " <ip:port>\t "
			"server with compression channel param\n"
		"\t" << g_client_with_compression_arg << " <ip:port>\t "
			"client  with compression channel param\n"
		"\t" << g_stagepoint_arg << " stage@endpoint\tcreate stage\n"
		"\t" << g_enpoint_arg << " endpoint[<stages>] target_endpoint\n"
			"\t\t\tcreate endpoint\n";
}

int
main( int argc, char ** argv )
{
	try
	{
		sample_mbapi_4::app_data_t app_data;
		size_t option_index = 1;

		while( option_index + 1 < argc )
		{
			if( argv[ option_index ] == g_server_arg )
			{
				app_data.m_server_addrs.push_back(
					std::make_pair( argv[ option_index + 1 ], false ) );
				option_index += 2;
			}
			else if( argv[ option_index ] == g_client_arg )
			{
				app_data.m_client_addrs.push_back(
					std::make_pair( argv[ option_index + 1 ], false ) );
				option_index += 2;
			}
			else if( argv[ option_index ] == g_server_with_compression_arg )
			{
				app_data.m_server_addrs.push_back(
					std::make_pair( argv[ option_index + 1 ], true ) );
				option_index += 2;
			}
			else if( argv[ option_index ] == g_client_with_compression_arg )
			{
				app_data.m_client_addrs.push_back(
					std::make_pair( argv[ option_index + 1 ], true ) );
				option_index += 2;
			}
			else if( argv[ option_index ] == g_stagepoint_arg )
			{
				app_data.m_stagepoints.push_back( argv[ option_index + 1 ] );
				option_index += 2;
			}
			else if( argv[ option_index ] == g_enpoint_arg )
			{
				if( option_index + 2 < argc )
				{
					sample_mbapi_4::endpoints_data_t endpoints_data;
					endpoints_data.m_ep_desc = argv[ option_index + 1 ];
					endpoints_data.m_target_endpoint = argv[ option_index + 2 ];

					app_data.m_endpoints.push_back( endpoints_data );
				}
				else
					break;
				option_index += 3;
			}
			else
				break;
		}

		if( option_index == argc )
		{
			return so_5::api::run_so_environment_on_object(
				app_data,
				&sample_mbapi_4::app_data_t::init,
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
		{
			std::cerr << "Arguments error.\n";
			print_usage( std::cout );
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
