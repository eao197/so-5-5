/*
	Замер скорости передачи сообщений, в случае передачи сообщения
	между двумя агентами, когда созданы множество привязок.
*/

#include <iostream>
#include <string>
#include <queue>
#include <cstdlib>

#include <ace/Time_Value.h>
#include <ace/OS.h>

#include <cpp_util_2/h/defs.hpp>
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

// Сообщений ping/pong.
#include <test/mbapi_4/bench3/h/messages.hpp>

namespace bench_mbapi_4
{

struct bind_wrapper_t
{
	bind_wrapper_t(
		mbapi_4::endpoint_bind_unique_ptr_t bind )
		:
			m_bind( std::move( bind ) )
	{}

	mbapi_4::endpoint_bind_unique_ptr_t m_bind;
};


typedef std::shared_ptr< bind_wrapper_t >
	bind_wrapper_ref_t;

typedef std::vector< bind_wrapper_ref_t > bind_wrapper_list_t;

// Сообщение сигнал о завершении теста.
struct msg_test_result_t
	:
		public so_5::rt::message_t
{
	// Время начале теста.
	ACE_Time_Value m_started;
	// Время завершения теста.
	ACE_Time_Value m_finished;

	// Количество полученных сообщений.
	size_t m_received_message_count;
};

inline ACE_UINT64
milliseconds( const ACE_Time_Value & tv )
{
	ACE_UINT64 v;
	tv.msec( v );
	return v;
}

// Агент, который начинает тест.
class a_test_starter_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public:
		a_test_starter_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::endpoint_stage_chain_t & self_endpoint,
			const mbapi_4::endpoint_t & target_endpoint,
			const so_5::rt::mbox_ref_t & mbox )
			:
				base_type_t( env ),
				m_self_endpoint( self_endpoint ),
				m_target_endpoint( target_endpoint ),
				m_mbox( mbox )
		{
			m_test_sizes.push( 1000 );
			m_test_sizes.push( 10 * 1000 );
			m_test_sizes.push( 20 * 1000 );
			m_test_sizes.push( 50 * 1000 );
			m_test_sizes.push( 100 * 1000 );
			m_test_sizes.push( 500 * 1000 );
			m_test_sizes.push( 1 * 1000 * 1000 );
			// m_test_sizes.push( 10 * 1000 * 1000 );
		}

		virtual ~a_test_starter_t()
		{}

		virtual void
		so_define_agent()
		{
			m_self_endpoint_bind = so_environment()
				.query_layer< mbapi_4::mbapi_layer_t >()
					->create_endpoint_bind( m_self_endpoint, *this );

			create_extra_endpoints();

			so_subscribe( m_mbox )
				.event( &a_test_starter_t::evt_test_result );
		}

		virtual void
		so_evt_start()
		{
			start_test();
		}

		void
		evt_test_result(
			const so_5::rt::event_data_t< msg_test_result_t > & msg )
		{
			const double work_time =
				milliseconds( msg->m_finished - msg->m_started ) / 1000.0;
			std::cout
				<< "messages: " << msg->m_received_message_count
				<< ", total time: " << std::fixed
					<< work_time << " sec"
				<< ", throughput: " << std::fixed
					<< msg->m_received_message_count / work_time << " msgs/sec\n"
				<< std::endl;

			start_test();
		}

	private:
		void
		start_test();

		void
		create_extra_endpoints();

		so_5::rt::mbox_ref_t m_mbox;
		std::queue< size_t > m_test_sizes;

		const mbapi_4::endpoint_stage_chain_t m_self_endpoint;
		const mbapi_4::endpoint_t m_target_endpoint;

		mbapi_4::endpoint_bind_unique_ptr_t
			m_self_endpoint_bind;

		// Дополнительные привязки.
		bind_wrapper_list_t m_binds;
};

void
a_test_starter_t::start_test()
{
	if( m_test_sizes.empty() )
	{
		so_environment().stop();
		return;
	}

	const size_t test_size = m_test_sizes.front();
	m_test_sizes.pop();

	m_self_endpoint_bind->send(
		m_target_endpoint,
		std::unique_ptr< start_test_t >(
			new start_test_t ) );

	for( size_t i = 0; i < test_size; ++i )
	{
		std::unique_ptr< request_t > request( new request_t( i ) );
		m_self_endpoint_bind->send(
			m_target_endpoint,
			std::move( request ) );
	}

	m_self_endpoint_bind->send(
		m_target_endpoint,
		std::unique_ptr< finish_test_t >(
			new finish_test_t ) );
}

std::string
create_ep_name( size_t num )
{
	std::string name_sample( "Aaaaa" );

	for( size_t i = 0; i < name_sample.size(); ++i )
	{
		name_sample[ i ] += rand() % 20;
	}

	return name_sample + "_" + cpp_util_2::slexcast( num );
}

void
a_test_starter_t::create_extra_endpoints()
{
	mbapi_4::mbapi_layer_t & mbapi_layer =
		*so_environment()
			.query_layer< mbapi_4::mbapi_layer_t >();

	for( size_t i = 0; i < 20*1000; ++i )
	{
		bind_wrapper_ref_t bind_wrapper_ref(
			new bind_wrapper_t(
				mbapi_layer.create_endpoint_bind(
					create_ep_name( i ), *this ) ) );
	}
}

//
// Класс агента, который принимает сообщения на заданной стадии.
//

class a_stage_tester_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public:
		a_stage_tester_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::stagepoint_t & self_stagepoint )
			:
				base_type_t( env ),
				m_self_stagepoint( self_stagepoint )
		{}

		virtual ~a_stage_tester_t()
		{}

		virtual void
		so_define_agent()
		{
			m_self_stagepoint_bind = so_environment()
				.query_layer< mbapi_4::mbapi_layer_t >()
					->create_stagepoint_bind( m_self_stagepoint, *this );

			m_self_stagepoint_bind->subscribe_event(
				so_default_state(),
				&a_stage_tester_t::evt_request );

			m_self_stagepoint_bind->subscribe_event(
				so_default_state(),
				&a_stage_tester_t::evt_start_test );

			m_self_stagepoint_bind->subscribe_event(
				so_default_state(),
				&a_stage_tester_t::evt_finish_test );
		}

		void
		evt_request(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< request_t > > & msg )
		{
			msg->move_next();
		}

		void
		evt_start_test(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< start_test_t > > & msg )
		{
			msg->move_next();
		}

		void
		evt_finish_test(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< finish_test_t > > & msg )
		{
			msg->move_next();
		}

	private:
		const mbapi_4::stagepoint_t m_self_stagepoint;

		mbapi_4::stagepoint_bind_unique_ptr_t
			m_self_stagepoint_bind;
};

//
// Класс агента, который принимает сообщения.
//

class a_tester_t
	:
		public so_5::rt::agent_t
{
	typedef so_5::rt::agent_t base_type_t;

	public:
		a_tester_t(
			so_5::rt::so_environment_t & env,
			const mbapi_4::endpoint_stage_chain_t & self_endpoint,
			const so_5::rt::mbox_ref_t mbox )
			:
				base_type_t( env ),
				m_self_endpoint( self_endpoint ),
				m_mbox( mbox )
		{}

		virtual ~a_tester_t()
		{}

		virtual void
		so_define_agent()
		{
			m_self_endpoint_bind = so_environment()
				.query_layer< mbapi_4::mbapi_layer_t >()
					->create_endpoint_bind( m_self_endpoint, *this );

			m_self_endpoint_bind->subscribe_event(
				so_default_state(),
				&a_tester_t::evt_request );

			m_self_endpoint_bind->subscribe_event(
				so_default_state(),
				&a_tester_t::evt_start_test );

			m_self_endpoint_bind->subscribe_event(
				so_default_state(),
				&a_tester_t::evt_finish_test );
		}

		void
		evt_request(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< request_t > > & msg )
		{
			const request_t & request = msg->msg();
			if( request.m_num != m_num_counter )
			{
				std::cerr << "message sequence interrupted: "
					"expected: " << m_num_counter << "; "
					"occured: " << request.m_num << std::endl;
				ACE_OS::abort();
			}
			++m_num_counter;
		}

		void
		evt_start_test(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< start_test_t > > & )
		{
			m_num_counter = 0;
			m_test_started = ACE_OS::gettimeofday();
		}

		void
		evt_finish_test(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< finish_test_t >  > & )
		{
			std::unique_ptr< msg_test_result_t >
				msg_test_result( new msg_test_result_t );

			msg_test_result->m_started = m_test_started;
			msg_test_result->m_finished = ACE_OS::gettimeofday();
			msg_test_result->m_received_message_count = m_num_counter;

			m_mbox->deliver_message( msg_test_result );
		}

	private:
		const mbapi_4::endpoint_stage_chain_t m_self_endpoint;

		mbapi_4::endpoint_bind_unique_ptr_t
			m_self_endpoint_bind;

		so_5::rt::mbox_ref_t m_mbox;

		unsigned int m_num_counter;
		ACE_Time_Value m_test_started;
};

void
add_ping_messages_oess_repository(
	so_5::rt::so_environment_t & env )
{
	env.query_layer< mbapi_4::mbapi_layer_t >()
		->add_repository( request_t::get_repository() );
}

void
init( so_5::rt::so_environment_t & env )
{
	add_ping_messages_oess_repository( env );

	// Создаем кооперацию
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "main_coop" ),
		so_5::disp::active_obj::create_disp_binder(
			"active_obj" ) );

	const so_5::rt::mbox_ref_t mbox = env.create_local_mbox();

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_test_starter_t(
				env,
				mbapi_4::endpoint_stage_chain_t::create_from_string(
					"Alpha[alpha_stage1, alpha_stage2, alpha_stage3]" ),
				mbapi_4::endpoint_t( "Betta" ),
				mbox ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"alpha_stage1@Alpha" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"alpha_stage2@Alpha" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"alpha_stage3@Alpha" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"betta_stage1@Betta" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"betta_stage2@Betta" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_stage_tester_t(
				env,
				mbapi_4::stagepoint_t::create_from_string(
					"betta_stage3@Betta" ) ) ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_tester_t(
				env,
				mbapi_4::endpoint_stage_chain_t::create_from_string(
					"Betta[betta_stage1, betta_stage2, betta_stage3]" ),
				mbox ) ) );

	// Регистрируем кооперацию.
	const so_5::ret_code_t rc = env.register_coop( std::move( coop ) );

	// Если не удается зарегистрировать кооперацию
	// напишем сообщение об ошибке.
	if( 0 != rc )
	{
		std::cerr << "reg coop ret_code = " << rc << "\n\n";
		throw std::runtime_error( "endpoint coop not registered" );
	}
}

} /* namespace bench_mbapi_4 */

int
main( int argc, char ** argv )
{
	try
	{
		return so_5::api::run_so_environment(
			&bench_mbapi_4::init,
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
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
