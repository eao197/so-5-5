/*
	Тестирование регистрации коопераций.

	Суть теста:
		Создается и регистрируется несколько коопераций.
		Затем часть из них дерегистрируется, а часть остается
		в системе до завершения работы SO.

		Агенты не подписываются ни на какие сообщения и ничего не делают.
*/

#include <iostream>
#include <map>
#include <exception>

#include <ace/Time_Value.h>
#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

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

		virtual void
		so_define_agent()
		{
			// Агент не подписывается ни на что.
		}

		virtual void
		so_evt_start()
		{
			// Ничего не делаем.
		}
};

void
reg_coop(
	const std::string & coop_name,
	so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop( coop_name );

	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );

	env.register_coop( std::move( coop ), so_5::THROW_ON_ERROR );
}

void
init( so_5::rt::so_environment_t & env )
{
	reg_coop( "test_coop_1", env );
	reg_coop( "test_coop_2", env );
	reg_coop( "test_coop_3", env );
	reg_coop( "test_coop_4", env );
	reg_coop( "test_coop_5", env );
	reg_coop( "test_coop_6", env );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000) );

	env.deregister_coop( "test_coop_1", so_5::THROW_ON_ERROR );

	env.deregister_coop( "test_coop_6", so_5::THROW_ON_ERROR );

	env.deregister_coop( "test_coop_3", so_5::THROW_ON_ERROR );

	ACE_OS::sleep( ACE_Time_Value( 0, 50*1000) );

	env.stop();
}


int
main( int argc, char * argv[] )
{
	try
	{
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
				.mbox_mutex_pool_size( 4 )
				.agent_event_queue_mutex_pool_size( 4 ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

