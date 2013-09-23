/*
 * A sample of simple agent which sends a periodic message to itself.
 */

#include <iostream>
#include <time.h>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Hello message.
class msg_hello_periodic
	:
		public so_5::rt::message_t
{
	public:
		// Greeting.
		std::string m_message;
};

// Stop message.
class msg_stop_signal : public so_5::rt::signal_t {};

// An agent class definition.
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

		// Definition of agent for SObjectizer.
		virtual void
		so_define_agent();

		// A reaction to start of work in SObjectizer.
		virtual void
		so_evt_start();

		// Hello message handler.
		void
		evt_hello_periodic(
			const so_5::rt::event_data_t< msg_hello_periodic > & msg );

		// Stop message handler.
		void
		evt_stop_signal(
			const so_5::rt::event_data_t< msg_stop_signal > & );

	private:
		// Agent's mbox.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Timer events' identifiers.
		so_5::timer_thread::timer_id_ref_t m_hello_timer_id;
		so_5::timer_thread::timer_id_ref_t m_stop_timer_id;

		// How much timer event has been processed.
		unsigned int m_evt_count;
};

void
a_hello_t::so_define_agent()
{
	// Subscription to messages.
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

	// Sending a greeting.
	m_hello_timer_id =
		so_environment()
			.schedule_timer< msg_hello_periodic >(
				std::move( msg ),
				m_self_mbox,
				// Delay for a second.
				1 * 1000,
				// Repeat every second.
				1 * 1000 );

	// Sending a stop signal.
	m_stop_timer_id =
		so_environment()
			.schedule_timer< msg_stop_signal >(
				m_self_mbox,
				// Delay for two seconds.
				2 * 1000,
				// Not a periodic.
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
		// Stops hello message.
		m_hello_timer_id.release();
	}
	else
	{
		// Reschedule stop signal.
		// Previous stop signal should be canceled.
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

	// Shutting down SObjectizer.
	so_environment().stop();
}

// SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	// Creating cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agent to cooperation.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env ) ) );

	// Registering cooperation.
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

