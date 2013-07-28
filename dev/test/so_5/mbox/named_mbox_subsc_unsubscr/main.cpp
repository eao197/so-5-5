/*
	Тестирование подписки отписки агента к именованному mbox-у.

	Суть теста:
		Создается кооперация с одним агентом.
		Агент подписывается на несколько сообщений от именованного mbox-а,
		который каждый раз получает с помощью
		so_environment_t::create_local_mbox(name).
		Затем агент отписывается от них с помощью таким же образом
		получаемого mbox-а.
		Далее высылает себе эти сообщения которые он не должен
		получить, и отсылает себе сигнал о завершении работы.
*/

#include <iostream>
#include <exception>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>


struct msg1{};
struct msg2{};
struct msg3{};
struct msg4{};
struct msg5{};

struct msg_stop{};

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

		virtual ~test_agent_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

#define ABORT_HANDLER( handler, msg ) \
	void\
	handler ( const so_5::rt::event_data_t< msg > & ) \
	{\
		std::cerr << "Error: " #msg " handler called..." << std::endl; \
		ACE_OS::abort(); \
	}
		ABORT_HANDLER( handler1, msg1 )
		ABORT_HANDLER( handler2, msg2 )
		ABORT_HANDLER( handler3, msg3 )
		ABORT_HANDLER( handler4, msg4 )
		ABORT_HANDLER( handler5, msg5 )

		void
		evt_stop(
			const so_5::rt::event_data_t< msg_stop > &
				msg )
		{
			so_environment().stop();
		}
};

void
test_agent_t::so_define_agent()
{
	so_5::rt::nonempty_name_t mbox_name( "test_mbox" );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::evt_stop );
}

void
test_agent_t::so_evt_start()
{
	so_5::rt::nonempty_name_t mbox_name( "test_mbox" );

	so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler1 );
	so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler2 );
	so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler3 );
	so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler4 );
	so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler5 );

	so_environment().create_local_mbox( mbox_name )->deliver_message< msg1 >();
	so_environment().create_local_mbox( mbox_name )->deliver_message< msg2 >();
	so_environment().create_local_mbox( mbox_name )->deliver_message< msg3 >();
	so_environment().create_local_mbox( mbox_name )->deliver_message< msg4 >();
	so_environment().create_local_mbox( mbox_name )->deliver_message< msg5 >();

	so_environment().create_local_mbox( mbox_name )->deliver_message< msg_stop >();
}

void
init(
	so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "test_coop" ) );

	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );

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



