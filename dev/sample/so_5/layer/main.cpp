/*
 * A demo of using custom layers.
 */

#include <iostream>
#include <set>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Main SObjectizer header files.
#include <so_5/api/h/api.hpp>
#include <so_5/rt/h/rt.hpp>
#include <so_5/rt/h/so_layer.hpp>

class msg_shutdown
	:
		public so_5::rt::message_t
{
	public:
		msg_shutdown()
		{}
		virtual ~msg_shutdown()
		{}
};

class msg_hello_to_all
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_all(
			const std::string & sender,
			const so_5::rt::mbox_ref_t & mbox )
			:
				m_sender( sender ),
				m_mbox( mbox )
		{}

		virtual ~msg_hello_to_all()
		{}

		// Sender name.
		std::string m_sender;
		// Sender mbox.
		so_5::rt::mbox_ref_t m_mbox;
};

class msg_hello_to_you
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_you(
			const std::string & sender )
			:
				m_sender( sender )
		{}
		virtual ~msg_hello_to_you()
		{}

		// Sender name.
		std::string m_sender;
};

class shutdowner_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:

		virtual ~shutdowner_layer_t()
		{
		}

		//
		// Implementation of methods for the layer control.
		//

		// Start layer.
		virtual void
		start()
		{
			m_shutdown_mbox = so_environment().create_local_mbox(
				so_5::rt::nonempty_name_t( "shutdown_mbox" ) );

			std::unique_ptr< msg_shutdown > msg( new msg_shutdown );

			so_environment().single_timer(
				std::move(msg),
				m_shutdown_mbox,
				3*1000 );
		}

		// Shutdown layer.
		virtual void
		shutdown()
		{
			std::cout << "shutdowner_layer shutdown()" << std::endl;
		}

		// Wait for the layer shutdown.
		virtual void
		wait()
		{
			std::cout << "shutdowner_layer wait()" << std::endl;
		}

		// Helper method for doing subscription.
		template< class AGENT >
		void
		subscribe_to_shutdown( AGENT & agent )
		{
			agent.subscribe( m_shutdown_mbox );
			m_subscribers.insert( &agent );
		}

		template< class AGENT >
		void
		unsubscribe( AGENT & agent )
		{
			std::set<so_5::rt::agent_t*>::iterator it =
				m_subscribers.find( &agent );

			if( it != m_subscribers.end() )

				m_subscribers.erase(it);

				if( m_subscribers.empty() )
				{
					std::cout << "all agents are unsubscribed\n";
					so_environment().stop();
				}
		}

	private:
		// Mbox for sending a shutdown signal.
		so_5::rt::mbox_ref_t m_shutdown_mbox;

		std::set< so_5::rt::agent_t* > m_subscribers;
};

// Definition of an agent for SObjectizer.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t(
			so_5::rt::so_environment_t & env,
			const std::string & agent_name )
			:
				base_type_t( env ),
				m_agent_name( agent_name ),
				m_self_mbox( so_environment().create_local_mbox() ),
				m_common_mbox( so_environment().create_local_mbox(
					so_5::rt::nonempty_name_t( "common_mbox" ) ) )
		{}
		virtual ~a_hello_t()
		{}

		// Definition of an agent for SObjectizer.
		virtual void
		so_define_agent();

		// A reaction to start of work in SObjectizer.
		virtual void
		so_evt_start();

		// A reaction to the common greeting.
		void
		evt_hello_to_all(
			const so_5::rt::event_data_t< msg_hello_to_all > & evt_data );

		// A reaction to the personal greeting.
		void
		evt_hello_to_you(
			const so_5::rt::event_data_t< msg_hello_to_you > & evt_data );

		// A reaction to the shutdown.
		void
		evt_shutdown(
			const so_5::rt::event_data_t< msg_shutdown > & evt_data );

		// A shutdown message subscription.
		// This method is a callback for the shutdowner_layer.
		void
		subscribe( so_5::rt::mbox_ref_t & shutdown_mbox );

	private:
		// Agent name.
		const std::string m_agent_name;

		// Agent mbox.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Common mbox.
		so_5::rt::mbox_ref_t m_common_mbox;

};

void
a_hello_t::so_define_agent()
{
	// Message subscription.
	so_subscribe( m_common_mbox )
		.event( &a_hello_t::evt_hello_to_all );

	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello_to_you );

	so_environment().query_layer< shutdowner_layer_t >()->
			subscribe_to_shutdown( *this );
}

void
a_hello_t::so_evt_start()
{
	std::cout << m_agent_name << ".so_evt_start" << std::endl;

	// Sending common greeting.
	m_common_mbox->deliver_message( std::unique_ptr< msg_hello_to_all >(
		new msg_hello_to_all(
			m_agent_name,
			m_self_mbox ) ) );
}

void
a_hello_t::evt_hello_to_all(
	const so_5::rt::event_data_t< msg_hello_to_all > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_all: "
		<< evt_data->m_sender << std::endl;

	// If we are not the sender then send personal greeting back.
	if( m_agent_name != evt_data->m_sender )
	{
		so_5::rt::mbox_ref_t mbox = evt_data->m_mbox;
		mbox->deliver_message( std::unique_ptr< msg_hello_to_you >(
			new msg_hello_to_you( m_agent_name ) ) );
	}
}

void
a_hello_t::evt_hello_to_you(
	const so_5::rt::event_data_t< msg_hello_to_you > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_you: "
		<< evt_data->m_sender << std::endl;
}

void
a_hello_t::evt_shutdown(
	const so_5::rt::event_data_t< msg_shutdown > & evt_data )
{
	std::cout << m_agent_name << ": preparing to shutdown\n";
	so_environment().query_layer< shutdowner_layer_t >()->unsubscribe(
		*this );
}

void
a_hello_t::subscribe( so_5::rt::mbox_ref_t & shutdown_mbox )
{
	std::cout << m_agent_name << ": subscription to shutdown\n";
	so_subscribe( shutdown_mbox )
		.event( &a_hello_t::evt_shutdown );
}

void
init( so_5::rt::so_environment_t & env )
{
	// Creating a cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agents to the cooperation.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "alpha" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "beta" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "gamma" ) ) );

	// Registering the cooperation.
	env.register_coop( std::move( coop ) );
}

int
main( int, char ** )
{
	try
	{
		so_5::api::run_so_environment(
				&init,
				so_5::rt::so_environment_params_t().add_layer(
						std::unique_ptr< shutdowner_layer_t >(
								new shutdowner_layer_t() ) ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
