/*
  Демострация обмена сообщениями.
*/

#include <iostream>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

class msg_hello_to_all
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_all(
			const std::string & sender,
			const so_5::rt::mbox_ref_t & mbox )
			:
				m_sender( sender ),
				m_mbox( mbox )
		{}

		// Имя отправителя.
		const std::string m_sender;
		// Mbox отправителя.
		const so_5::rt::mbox_ref_t m_mbox;
};

class msg_hello_to_you
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_you(
			const std::string & sender )
			:
				m_sender( sender )
		{}

		// Имя отправителя.
		const std::string m_sender;
};


// C++ описание класса агента.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t(
			so_5::rt::so_environment_t & env,
			const std::string & agent_name )
			:
				base_type_t( env ),
				m_agent_name( agent_name ),
				m_self_mbox( so_environment().create_local_mbox() ),
				m_common_mbox( so_environment().create_local_mbox(
					so_5::rt::nonempty_name_t( "common_mbox" ) ) )
		{}
		virtual ~a_hello_t()
		{}

		// Определение агента.
		virtual void
		so_define_agent();

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start();

		void
		evt_hello_to_all(
			const so_5::rt::event_data_t< msg_hello_to_all > & evt_data );

		void
		evt_hello_to_you(
			const so_5::rt::event_data_t< msg_hello_to_you > & evt_data );

	private:
		// Имя агента.
		const std::string m_agent_name;

		// Mbox данного агента.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Общий mbox для всех агентов.
		so_5::rt::mbox_ref_t m_common_mbox;
};

void
a_hello_t::so_define_agent()
{
	// Подписываемся на сообщения.
	so_subscribe( m_common_mbox )
		.event( &a_hello_t::evt_hello_to_all );

	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello_to_you );
}

void
a_hello_t::so_evt_start()
{
	std::cout << m_agent_name << ".so_evt_start" << std::endl;

	// Отсылаем всем приветствие.
	m_common_mbox->deliver_message( std::unique_ptr< msg_hello_to_all >(
		new msg_hello_to_all(
			m_agent_name,
			m_self_mbox ) ) );
}

void
a_hello_t::evt_hello_to_all(
	const so_5::rt::event_data_t< msg_hello_to_all > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_all: "
		<< evt_data->m_sender << std::endl;

	// Если привествие слали не мы, то отошлем ответ.
	if( m_agent_name != evt_data->m_sender )
	{
		so_5::rt::mbox_ref_t mbox = evt_data->m_mbox;
		mbox->deliver_message( std::unique_ptr< msg_hello_to_you >(
			new msg_hello_to_you( m_agent_name ) ) );
	}
}

void
a_hello_t::evt_hello_to_you(
	const so_5::rt::event_data_t< msg_hello_to_you > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_you: "
		<< evt_data->m_sender << std::endl;
}

void
init( so_5::rt::so_environment_t & env )
{
	// Создаем кооперацию.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Добавляем в кооперацию агентов.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "alpha" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "beta" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "gamma" ) ) );

	// Регистрируем кооперацию.
	env.register_coop( std::move( coop ) );

	// Даем время агентам обменяться сообщениями.
	ACE_OS::sleep( ACE_Time_Value( 0, 200*1000 ) );
	env.stop();
}

int
main( int, char ** )
{
	try
	{
		so_5::api::run_so_environment( &init );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
