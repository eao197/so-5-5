/*
	Тестирование регистрации коопераций.

	Суть теста:
		Создается и регистрируется несколько коопераций.
		Агентам отправляется сообщение.
		Затем часть коопераций дерегистрируется.
		И снова агентам отправляется сообшение.

		Агенты подписываются на одно сообщение.

*/

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>

#include <ace/OS.h>
#include <ace/Time_Value.h>

#include <so_5/h/types.hpp>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

so_5::rt::nonempty_name_t g_test_mbox_name( "test_mbox" );

struct test_message
	:
		public so_5::rt::message_t
{
	test_message() {}
	virtual ~test_message(){}
};

class test_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:

		test_agent_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & test_mbox )
			:
				base_type_t( env ),
				m_test_mbox( test_mbox )
		{
			++m_agent_count;
		}

		virtual ~test_agent_t()
		{
			--m_agent_count;
		}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start()
		{
			// Ничего не делаем.
		}

		void
		evt_test(
			const so_5::rt::event_data_t< test_message > &
				msg );

		// Количество живых агентов.
		static so_5::atomic_counter_t m_agent_count;

		// Количество вызовов обработчика событий.
		static so_5::atomic_counter_t m_message_rec_cnt;

	private:
		so_5::rt::mbox_ref_t m_test_mbox;

};

so_5::atomic_counter_t test_agent_t::m_agent_count = 0;
so_5::atomic_counter_t test_agent_t::m_message_rec_cnt = 0;

void
test_agent_t::so_define_agent()
{
	so_subscribe( m_test_mbox )
		.in( so_default_state() )
			.event(
				&test_agent_t::evt_test,
				so_5::THROW_ON_ERROR );
}

void
test_agent_t::evt_test(
	const so_5::rt::event_data_t< test_message > &
		msg )
{
	++m_message_rec_cnt;
}
void
reg_coop(
	const std::string & coop_name,
	const so_5::rt::mbox_ref_t & test_mbox,
	so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop =
		env.create_coop( coop_name );

	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env, test_mbox ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env, test_mbox ) ) );

	env.register_coop( std::move( coop ), so_5::THROW_ON_ERROR );
}

void
init( so_5::rt::so_environment_t & env )
{
	so_5::rt::mbox_ref_t test_mbox =
		env.create_local_mbox( g_test_mbox_name );

	reg_coop( "test_coop_1", test_mbox, env );
	reg_coop( "test_coop_2", test_mbox, env );
	reg_coop( "test_coop_3", test_mbox, env );
	reg_coop( "test_coop_4", test_mbox, env );
	reg_coop( "test_coop_5", test_mbox, env );
	reg_coop( "test_coop_6", test_mbox, env );

	// Дадим агентам время подписаться.
	ACE_OS::sleep( ACE_Time_Value( 0, 100*1000) );

	env.create_local_mbox( g_test_mbox_name )
		->deliver_message< test_message >();

	// Даем выполниться событиям
	ACE_OS::sleep( ACE_Time_Value( 0, 100*1000) );

	unsigned int x = test_agent_t::m_agent_count.value();
	x -= test_agent_t::m_message_rec_cnt.value();
	if( 0 != x )
		throw std::runtime_error(
			"check 1: test_agent_t::m_agent_count != test_agent_t::m_message_rec_cnt" );

	env.deregister_coop( so_5::rt::nonempty_name_t(
		"test_coop_1" ), so_5::THROW_ON_ERROR );

	env.deregister_coop( so_5::rt::nonempty_name_t(
		"test_coop_6" ), so_5::THROW_ON_ERROR );

	env.deregister_coop( so_5::rt::nonempty_name_t(
		"test_coop_3" ), so_5::THROW_ON_ERROR );

	test_agent_t::m_message_rec_cnt = 0;

	ACE_OS::sleep( ACE_Time_Value( 0, 100*1000) );
	env.create_local_mbox( g_test_mbox_name )
		->deliver_message< test_message >();

	// Даем выполниться событиям
	ACE_OS::sleep( ACE_Time_Value( 0, 100*1000) );

	x = test_agent_t::m_agent_count.value();
	x -= test_agent_t::m_message_rec_cnt.value();
	if( 0 != x )
		throw std::runtime_error(
			"check 2: test_agent_t::m_agent_count != test_agent_t::m_message_rec_cnt" );

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
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
