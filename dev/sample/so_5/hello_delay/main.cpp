/*
 * A sample of a simple agent which send a delayed message to itself.
 */

#include <iostream>
#include <time.h>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Hello message.
class msg_hello
	:
		public so_5::rt::message_t
{
	public:
		// Greeting.
		std::string m_message;
};

// Stop message.
class msg_stop_signal : public so_5::rt::signal_t {};

// An agent class.
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

		// Definition of an agent for SObjectizer.
		virtual void
		so_define_agent();

		// A reaction to start of work in SObjectizer.
		virtual void
		so_evt_start();

		// Delayed message handler.
		void
		evt_hello_delay(
			const so_5::rt::event_data_t< msg_hello > & msg );

		// Stop signal handler.
		void
		evt_stop_signal(
			const so_5::rt::event_data_t< msg_stop_signal > & );

	private:
		// Agent's mbox.
		so_5::rt::mbox_ref_t	m_self_mbox;
};

void
a_hello_t::so_define_agent()
{
	// Message subscription.
	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello_delay );

	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_stop_signal );
}

void
a_hello_t::so_evt_start()
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "a_hello_t::so_evt_start()" << std::endl;

	std::unique_ptr< msg_hello > msg( new msg_hello );
	msg->m_message = "Hello, world! This is SObjectizer v.5.";

	// Send greeting.
	so_environment().single_timer(
		std::move( msg ),
		m_self_mbox,
		// Delay for 2 seconds.
		2*1000 );
}

void
a_hello_t::evt_hello_delay(
	const so_5::rt::event_data_t< msg_hello > & msg )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< msg->m_message << std::endl;

	so_environment().single_timer< msg_stop_signal >(
		m_self_mbox,
		2*1000 );
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

// The SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	// Creating a cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding an agent.
	coop->add_agent( new a_hello_t( env ) );

	// Registering the cooperation.
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

