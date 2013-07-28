/*
	Пример агента владеющего двумя endpoint-ами, который
	посылает себе с одного endpoint-а сообщение на другой endpoint.
*/

#include <iostream>
#include <algorithm>
#include <string>

#include <oess_2/defs/h/types.hpp>
#include <oess_2/stdsn/h/serializable.hpp>
#include <oess_2/stdsn/h/ent_std.hpp>
#include <oess_2/stdsn/h/inout_templ.hpp>
#include <oess_2/stdsn/h/shptr.hpp>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/h/mbapi_layer.hpp>
#include <mbapi_4/h/endpoint_bind.hpp>
#include <mbapi_4/h/message.hpp>

namespace sample_mbapi_4
{

struct sample_message_t
	:
		public oess_2::stdsn::serializable_t
{
	OESS_SERIALIZER( sample_message_t )

	public:
		std::string m_message;
		oess_2::uint_t m_counter;
};

std::string
hex_digit( unsigned char c )
{
	static const std::string hex_table = "0123456789abcdef";
	return hex_table.substr( c >> 4, 1 ) + hex_table.substr( c & 0xF, 1 );
}

std::string
create_hex_dump(
	const std::string & bin )
{
	std::string res;
	std::for_each(
		bin.begin(),
		bin.end(),
		[&]( unsigned char c ){ res += hex_digit( c ); } );

	return res;
}

// C++ описание класса агента.
class a_main_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_main_t( so_5::rt::so_environment_t & env )
			:
				base_type_t( env )
		{}

		virtual ~a_main_t()
		{}

		// Определение агента.
		virtual void
		so_define_agent();

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start()
		{
			std::cout << "so_evt_start()\n\n";

			std::unique_ptr< sample_message_t > new_msg(
				new sample_message_t );

			new_msg->m_counter = 5;
			new_msg->m_message = "Hello!";
			m_ep_b->send( m_ep_a->endpoint(), new_msg );
		}

		// Обработчик сообщения.
		void
		evt_sample_message(
			const so_5::rt::event_data_t<
				mbapi_4::message_t< sample_message_t > > & msg )
		{
			std::cout << "\nevt_sample_message:\n"
				"  from: " << msg->from().name() << "\n"
				"  to: " << msg->to().name() << "\n"
				"  message: \"" << msg->msg().m_message << "\"\n"
				"  counter: " << msg->msg().m_counter << "\n";

			if( 0 == msg->msg().m_counter )
				so_environment().stop();
			else
			{
				std::unique_ptr< sample_message_t > new_msg(
					new sample_message_t );

				new_msg->m_counter = msg->msg().m_counter - 1;
				new_msg->m_message = msg->msg().m_message + " Hello!";
				m_ep_a->send( m_ep_b->endpoint(), new_msg );
			}
		}

		// Обработчик сообщения.
		void
		evt_sample_binary_message(
			const so_5::rt::event_data_t<
				mbapi_4::binary_message_t > & msg )
		{
			std::cout << "\nevt_sample_binary_message:\n"
				"  from: " << msg->from().name() << "\n"
				"  to: " << msg->to().name() << "\n"
				"  bin: " << create_hex_dump( msg->m_bin_message ) << "\n";

			m_ep_b->send_binary_message(
				m_ep_a->endpoint(),
				msg->m_oess_id_wrapper,
				msg->m_bin_message );
		}

	private:
		// Endpoint-ы данного агента.
		mbapi_4::endpoint_bind_unique_ptr_t m_ep_a;
		mbapi_4::endpoint_bind_unique_ptr_t m_ep_b;
};

void
a_main_t::so_define_agent()
{
	m_ep_a = so_environment()
		.query_layer< mbapi_4::mbapi_layer_t >()
			->create_endpoint_bind(
				mbapi_4::endpoint_t( "sample_mbapi_4-A" ),
				*this );

	m_ep_b = so_environment()
		.query_layer< mbapi_4::mbapi_layer_t >()
			->create_endpoint_bind(
				mbapi_4::endpoint_t( "sample_mbapi_4-B" ),
				*this );

	m_ep_a->subscribe_event(
		so_default_state(),
		&a_main_t::evt_sample_message );

	m_ep_b->subscribe_event(
		so_default_state(),
		&a_main_t::evt_sample_binary_message );
}

// Инициализация окружения
void
init( so_5::rt::so_environment_t & env )
{
	std::cout << "1 init()\n\n";

	// Создаем кооперацию.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "sample_mbapi_4::main_coop" ) );

	// Добавляем в кооперацию агента.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_main_t( env ) ) );

	// Регистрируем кооперацию.
	env.register_coop( std::move( coop ) );
	std::cout << "2 init()\n\n";
}

} /* namespace sample_mbapi_4 */

#include "messages.ddl.cpp"

int
main( int, char ** )
{
	try
	{
		return so_5::api::run_so_environment(
			&sample_mbapi_4::init,
			so_5::rt::so_environment_params_t()
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
