/*
	Пример простейшего агента, который сама себе отправляет сообщение.
*/

#include <iostream>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Сообщение содержащее приветствие.
class msg_hello
	:
		public so_5::rt::message_t
{
	public:
		// Строка с приветствием.
		std::string m_message;
};

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

		// Обработчик сообщения msg_hello.
		void
		evt_hello(
			const so_5::rt::event_data_t< msg_hello > & msg );

	private:
		// Mbox данного агента.
		so_5::rt::mbox_ref_t m_self_mbox;
};

void
a_hello_t::so_define_agent()
{
	// Подписываемся на сообщения.
	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello );
}

void
a_hello_t::so_evt_start()
{
	std::unique_ptr< msg_hello > msg( new msg_hello );
	msg->m_message = "Hello, world! This is SObjectizer v.5.";

	// Отсылаем сообщение для вывода приветствия.
	m_self_mbox->deliver_message( msg );
}

void
a_hello_t::evt_hello(
	const so_5::rt::event_data_t< msg_hello > & msg )
{
	std::cout << msg->m_message << std::endl;

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

