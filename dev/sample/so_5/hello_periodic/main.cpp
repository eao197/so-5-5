/*
	Пример простейшего агента, который посылает себе
	периодическое сообщение.
*/

#include <iostream>
#include <time.h>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Сообщение о необходимости выдать приветствие.
class msg_hello_periodic
	:
		public so_5::rt::message_t
{
	public:
		// Сообщение с приветствием.
		std::string m_message;
};

class msg_stop_signal
	:
		public so_5::rt::message_t
{};

// C++ описание класса агента.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t( so_5::rt::so_environment_t & env )
			:
				base_type_t( env ),
				m_evt_count( 0 ),
				m_self_mbox( so_environment().create_local_mbox() )
		{}
		virtual ~a_hello_t()
		{}

		// Определение агента.
		virtual void
		so_define_agent();

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start();

		// Обработчик сообщения msg_hello_periodic.
		void
		evt_hello_periodic(
			const so_5::rt::event_data_t< msg_hello_periodic > & msg );

		// Обработчик сигнала о завершении работы.
		void
		evt_stop_signal(
			const so_5::rt::event_data_t< msg_stop_signal > & );

	private:
		// Mbox данного агента.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Таймерные идентификаторы для периодических сообщений.
		so_5::timer_thread::timer_id_ref_t m_hello_timer_id;
		so_5::timer_thread::timer_id_ref_t m_stop_timer_id;

		// Количество обработок периодического сообщения.
		unsigned int m_evt_count;
};

void
a_hello_t::so_define_agent()
{
	// Подписываемся на сообщения.
	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello_periodic );

	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_stop_signal );
}

void
a_hello_t::so_evt_start()
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "a_hello_t::so_evt_start()" << std::endl;

	std::unique_ptr< msg_hello_periodic > msg( new msg_hello_periodic );
	msg->m_message = "Hello, periodic!";

	// Отсылаем приветствие.
	m_hello_timer_id =
		so_environment()
			.schedule_timer< msg_hello_periodic >(
				std::move( msg ),
				m_self_mbox,
				1 * 1000,
				1 * 1000 );

	// Отсылаем отложенное сообщение-сигнал
	// для завершения работы.
	m_stop_timer_id =
		so_environment()
			.schedule_timer< msg_stop_signal >(
				m_self_mbox,
				2 * 1000,
				0 );
}

void
a_hello_t::evt_hello_periodic(
	const so_5::rt::event_data_t< msg_hello_periodic > & msg )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< msg->m_message << std::endl;

	if( 5 == ++m_evt_count )
	{
		// Отменяем таймерное событие.
		m_hello_timer_id.release();
	}
	else
	{
		// Переназначаем отложенную отправку
		// сообщения-сигнала о завершении работы.
		// Предшествующая отправка должна быть отменена.
		m_stop_timer_id =
			so_environment()
				.schedule_timer< msg_stop_signal >(
					m_self_mbox,
					2 * 1000,
					0 );
	}
}

void
a_hello_t::evt_stop_signal(
	const so_5::rt::event_data_t< msg_stop_signal > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "Stop SObjectizer..." << std::endl;

	// Завершаем работу.
	so_environment().stop();
}

// Инициализация окружения
void
init( so_5::rt::so_environment_t & env )
{
	// Создаем кооперацию.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Добавляем в кооперацию агента.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env ) ) );

	// Регистрируем кооперацию.
	env.register_coop( std::move( coop ) );
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
