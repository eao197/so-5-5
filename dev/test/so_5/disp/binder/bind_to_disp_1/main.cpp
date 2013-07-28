/*
	Тестирование привязок к диспетчерам.

	Суть теста:
		Создается и регистрируется кооперация,
		агенты которой привязываются к разным диспетчерам.
		Затем агент отправитель - шлет сообщения,
		агенту принимателю.

*/

#include <iostream>
#include <exception>
#include <stdexcept>
#include <memory>

#include <ace/OS.h>
#include <ace/Time_Value.h>

#include <so_5/h/types.hpp>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

#include <so_5/disp/one_thread/h/pub.hpp>

// По сколько сообщений отправлять за раз.
const unsigned int g_send_at_once = 10;

// Сколько раз отправители будут отпралять сообщения.
const unsigned int g_send_session_count = 100;

struct test_message
	:
		public so_5::rt::message_t
{
	test_message(): m_is_last( false ) {}
	virtual ~test_message() {}

	bool m_is_last;
};

// Сообщение для посылки сообщений отправителем
struct send_message_signal
	:
		public so_5::rt::message_t
{
	send_message_signal() {}
	virtual ~send_message_signal() {}
};

class test_agent_sender_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:

		test_agent_sender_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & mbox )
			:
				base_type_t( env ),
				m_send_session_complited( 0 ),
				m_mbox_receiver( mbox ),
				m_notification_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~test_agent_sender_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

		void
		evt_send_messages(
			const so_5::rt::event_data_t< send_message_signal > &
				msg );

	private:
		// Какая по счету это отправка.
		unsigned int m_send_session_complited;

		// Куда отправлять сообщения?
		so_5::rt::mbox_ref_t m_mbox_receiver;

		// Mbox для отсылки сообщений себе.
		so_5::rt::mbox_ref_t m_notification_mbox;
};

void
test_agent_sender_t::so_define_agent()
{
	so_subscribe( m_notification_mbox )
		.event(
			&test_agent_sender_t::evt_send_messages,
			so_5::THROW_ON_ERROR );
}

void
test_agent_sender_t::so_evt_start()
{
	m_notification_mbox->deliver_message< send_message_signal >();
}

void
test_agent_sender_t::evt_send_messages(
	const so_5::rt::event_data_t< send_message_signal > &
		msg )
{
	for( int i = 0; i < g_send_at_once; ++i )
	{
		m_mbox_receiver->deliver_message(
			std::unique_ptr< test_message >(
				new test_message ) );
	}

	++m_send_session_complited;

	// Если все сессии отправки сообщений завершили, то
	// шлем последнее сообщение.
	if( g_send_session_count <= m_send_session_complited )
	{
		std::unique_ptr< test_message > tm( new test_message );
		tm->m_is_last = true;

		m_mbox_receiver->deliver_message( tm );
	}
	else
	{
		m_notification_mbox->deliver_message< send_message_signal >();
	}
}

class test_agent_receiver_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		test_agent_receiver_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & mbox )
			:
				base_type_t( env ),
				m_source_mbox( mbox )
		{}

		virtual ~test_agent_receiver_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start()
		{}

		void
		evt_test(
			const so_5::rt::event_data_t< test_message > &
				msg );

	private:
		// Откуда принимать сообщения?
		so_5::rt::mbox_ref_t m_source_mbox;
};

void
test_agent_receiver_t::so_define_agent()
{
	so_subscribe( m_source_mbox )
		.event(
			&test_agent_receiver_t::evt_test,
			so_5::THROW_ON_ERROR );
}

void
test_agent_receiver_t::evt_test(
	const so_5::rt::event_data_t< test_message > &
		msg )
{
	// Если это последнее сообщение то останавливаемся.
	if( msg->m_is_last )
		so_environment().stop();
}

void
init( so_5::rt::so_environment_t & env )
{
	so_5::rt::mbox_ref_t mbox = env.create_local_mbox();

	so_5::rt::agent_coop_unique_ptr_t coop =
		env.create_coop( "test_coop" );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new test_agent_sender_t( env, mbox ) ),
			so_5::disp::one_thread::create_disp_binder(
				"sender_disp" ) );

	coop->add_agent(
		so_5::rt::agent_ref_t(
			new test_agent_receiver_t( env, mbox ) ),
			so_5::disp::one_thread::create_disp_binder(
				"receiver_disp" ) );

	env.register_coop(
		std::move( coop ),
		so_5::THROW_ON_ERROR );
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
				.agent_event_queue_mutex_pool_size( 4 )
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "sender_disp" ),
					so_5::disp::one_thread::create_disp() )
				.add_named_dispatcher(
					so_5::rt::nonempty_name_t( "receiver_disp" ),
					so_5::disp::one_thread::create_disp() ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
