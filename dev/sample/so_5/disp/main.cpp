/*
	Пример работы с диспетчерами.
*/

#include <iostream>
#include <sstream>

#include <ace/OS.h>
#include <ace/Log_Msg.h>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Диспетчеры.
#include <so_5/disp/one_thread/h/pub.hpp>
#include <so_5/disp/active_group/h/pub.hpp>
#include <so_5/disp/active_obj/h/pub.hpp>

// C++ описание класса агента.
class a_disp_user_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_disp_user_t(
			so_5::rt::so_environment_t & env,
			const std::string & name )
			:
				base_type_t( env ),
				m_name( name )
		{}
		virtual ~a_disp_user_t()
		{}

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start();

		// Обработка завершения работы агента в системе.
		virtual void
		so_evt_finish();

	private:
		const std::string m_name;
};

// Макрос для формата сообщений, выводимых с помощью ACE logging.
#define AGENT_MSG( s ) "TID:%t %T " s

void
a_disp_user_t::so_evt_start()
{
	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_start(): start pause\n" ), m_name.c_str() ));

	// Засыпаем на одну секунду
	ACE_OS::sleep( 1 );

	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_start(): finish pause\n" ), m_name.c_str() ));
}

void
a_disp_user_t::so_evt_finish()
{
	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_finish(): start pause\n" ), m_name.c_str() ));

	// Засыпаем на одну секунду
	ACE_OS::sleep( 1 );

	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_finish(): finish pause\n" ), m_name.c_str() ));
}

// Создать имя агента.
std::string
create_agent_name( const std::string & base, int i )
{
	std::ostringstream sout;
	sout << base << "_" << i;

	return sout.str();
}


// Инициализация окружения
void
init( so_5::rt::so_environment_t & env )
{
	// Создаем кооперацию.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Добавляем в кооперацию агентов, которые будут работать
	// с диспетчером SObjectizer по умолчанию.
	for( int i = 0; i < 4; ++i )
	{
		const std::string name = create_agent_name( "default_disp", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t(
				new a_disp_user_t( env, name ) ) );
	}

	// Добавляем в кооперацию агентов, которые будут работать
	// с диспетчером single_thread.
	for( int i = 0; i < 3; ++i )
	{
		const std::string name = create_agent_name( "single_thread", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::one_thread::create_disp_binder(
				"single_thread" ) );
	}

	// Добавляем в кооперацию агентов, которые будут работать
	// с диспетчером active_group в группе A.
	for( int i = 0; i < 2; ++i )
	{
		const std::string name = create_agent_name( "active_group_A", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_group::create_disp_binder(
				"active_group",
				"A" ) );
	}

	// Добавляем в кооперацию агентов, которые будут работать
	// с диспетчером active_group в группе B.
	for( int i = 0; i < 2; ++i )
	{
		const std::string name = create_agent_name( "active_group_B", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_group::create_disp_binder(
				"active_group",
				"B" ) );
	}

	// Добавляем в кооперацию агентов, которые будут работать
	// с диспетчером active_object.
	for( int i = 0; i < 4; ++i )
	{
		const std::string name = create_agent_name( "active_obj", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );
	}

	// Регистрируем кооперацию.
	env.register_coop( std::move( coop ) );

	// Инициируем завершение.
	env.stop();
}

int
main( int, char ** argv )
{
	ACE_LOG_MSG->open (argv[0], ACE_Log_Msg::STDERR);
	try
	{
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "single_thread" ),
					so_5::disp::one_thread::create_disp() )
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "active_group" ),
					so_5::disp::active_group::create_disp() )
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "active_obj" ),
					so_5::disp::active_obj::create_disp() ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
