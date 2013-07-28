/*
	Тестирование регистрации коопераций.

	Суть теста:
		Создается и регистрируется кооперпация,
		после чего создается кооперация с таким же именем.
		Кооперация с повторяющимся именем, не должна быть зарегистрирована.

		Агенты не подписываются ни на какие сообщения и ничего не делают.
*/

#include <iostream>
#include <map>
#include <exception>

#include <ace/OS.h>
#include <ace/Time_Value.h>

#include <so_5/api/h/api.hpp>
#include <so_5/rt/h/rt.hpp>
#include <so_5/h/ret_code.hpp>

class test_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		test_agent_t(
			so_5::rt::so_environment_t & env )
			:
				base_type_t( env )
		{}

		virtual ~test_agent_t() {}

		// virtual void
		// so_define_agent()
		// {
			// // Агент не подписывается ни на что.
		// }
};

void
init( so_5::rt::so_environment_t & env )
{
	const std::string coop_name( "main_coop" );
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( coop_name ) );

	coop->add_agent(
		so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
	env.register_coop( std::move( coop) , so_5::THROW_ON_ERROR );

	// Создаем дубликат с таким же именем.
	coop = env.create_coop( so_5::rt::nonempty_name_t( coop_name ) );
	coop->add_agent(
		so_5::rt::agent_ref_t( new test_agent_t( env ) ) );

	const so_5::ret_code_t rc =
		env.register_coop( std::move( coop ), so_5::DO_NOT_THROW_ON_ERROR );

	if( so_5::rc_coop_with_specified_name_is_already_registered != rc )
		throw std::runtime_error( "duplicating coop should not be registered" );

	env.stop();
}

int
main( int, char ** )
{
	try
	{
		so_5::api::run_so_environment(
			&init );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}



