#include <iostream>
#include <string>

#include <ace/OS.h>

#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/local_mbox.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/so_environment.hpp>

struct my_message
	:
		public so_5::rt::message_t
{
	my_message() {}
	virtual ~my_message(){}

	int x;
};

struct my_unsubscribe_signal
	:
		public so_5::rt::message_t
{
};

struct my_another_message
	:
		public so_5::rt::message_t
{
	my_another_message() {}
	virtual ~my_another_message(){}

	std::string s;
};


class my_agent_t
	:
		public so_5::rt::agent_t
{
		const so_5::rt::state_t m_first_state;
		const so_5::rt::state_t m_second_state;
	public:

		my_agent_t(
			const so_5::rt::mbox_ref_t & mbox_ref )
			:
				m_first_state( self_ptr() ),
				m_second_state( self_ptr() ),
				m_mbox_ref( mbox_ref )
		{}

		virtual ~my_agent_t() {}

		void
		my_event_handler(
			const so_5::rt::event_data_t< my_message > &
				message );

		void
		my_another_event_handler(
			const so_5::rt::event_data_t< my_another_message > &
				message );

		virtual void
		so_define_agent();

		virtual void
		so_evt_start()
		{
			std::cout << "so_evt_start()!!\n";
			m_mbox_ref->deliver_message< my_unsubscribe_signal >();
		}

		void
		unsubscribe_event_handler(
			const so_5::rt::event_data_t< my_unsubscribe_signal > &
				message );

	private:
		so_5::rt::mbox_ref_t m_mbox_ref;
};

void
my_agent_t::so_define_agent()
{
	// Регистрация

	so_subscribe( m_mbox_ref )
		.in( m_first_state )
			.event( &my_agent_t::my_event_handler );

	so_subscribe( m_mbox_ref )
		.in( m_first_state )
			.event( &my_agent_t::my_another_event_handler );

	so_subscribe( m_mbox_ref )
		.in( m_second_state )
			.event( &my_agent_t::my_event_handler );

	so_subscribe( m_mbox_ref )
		.in( m_second_state )
			.event( &my_agent_t::my_another_event_handler );

	so_subscribe( m_mbox_ref )
		.in( so_default_state() )
			.event( &my_agent_t::unsubscribe_event_handler );
}

void
my_agent_t::unsubscribe_event_handler(
	const so_5::rt::event_data_t< my_unsubscribe_signal > &
		message )
{
	std::cout << "unsubscribe_event_handler()!!\n";
	// Дерегистрация

	so_unsubscribe( m_mbox_ref )
	.in( m_first_state )
		.event( &my_agent_t::my_event_handler );

	so_unsubscribe( m_mbox_ref )
	.in( m_first_state )
		.event( &my_agent_t::my_another_event_handler );

	so_unsubscribe( m_mbox_ref )
	.in( m_second_state )
		.event( &my_agent_t::my_event_handler );

	so_unsubscribe( m_mbox_ref )
	.in( m_second_state )
		.event( &my_agent_t::my_another_event_handler );
}

void
my_agent_t::my_event_handler(
	const so_5::rt::event_data_t< my_message > &
		message )
{
	std::cout << " my_agent_t::my_method:\t"
		<< "message.x = " << message->x << "\n";
}

void
my_agent_t::my_another_event_handler(
	const so_5::rt::event_data_t< my_another_message > &
		message )
{
	std::cout << " my_agent_t::my_another_event_handler:\t"
		<< "message.s = '" << message->s << "'\n";
}


class so_environment_t
	:
		public so_5::rt::so_environment_t
{
		typedef so_5::rt::so_environment_t base_type_t;
	public:
		so_environment_t()
			:
				base_type_t( so_5::rt::so_environment_params_t()
					.mbox_mutex_pool_size( 16 ) ),
				m_test_mbox( create_local_mbox(
					so_5::rt::nonempty_name_t( "test_mbox" ) ) )
		{}

		virtual ~so_environment_t(){}

		virtual void
		init()
		{
			so_5::rt::agent_coop_unique_ptr_t coop = create_coop(
				so_5::rt::nonempty_name_t( "first_coop" ) );

			coop->add_agent( so_5::rt::agent_ref_t(
				new my_agent_t( m_test_mbox ) ) );

			so_5::ret_code_t rc = register_coop( coop );
			std::cout << "\nrc = " << rc << "\n\n";

			ACE_OS::sleep( 10 );
			stop();
		}

	private:
		so_5::rt::mbox_ref_t	m_test_mbox;

};

int
main( int, char ** )
{
	std::cout << "test mboxes \n\n";

	so_environment_t so_5_env;
	so_5_env.run();

	return 0;
}
