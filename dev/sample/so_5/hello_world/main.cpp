/*
	Пример простейшего агента.
*/

#include <iostream>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// C++ описание класса агента.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t( so_5::rt::so_environment_t & env )
			:
				 // Передаем среду SObjectizer, к которой принадлежит агент.
				base_type_t( env )
		{}
		virtual ~a_hello_t()
		{}

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start()
		{
			std::cout << "Hello, world! This is SObjectizer v.5."
				<< std::endl;

			// Завершаем работу примера.
			so_environment().stop();
		}

		// Обработка завершения работы агента в системе.
		virtual void
		so_evt_finish()
		{
			std::cout << "Bye! This was SObjectizer v.5."
				<< std::endl;
		}
};

// Инициализация окружения
void
init( so_5::rt::so_environment_t & env )
{
	// Создаем кооперацию.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Добавляем в кооперацию агента.
	coop->add_agent(
		so_5::rt::agent_ref_t(
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
