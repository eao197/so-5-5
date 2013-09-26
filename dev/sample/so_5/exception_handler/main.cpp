/*
 * A sample for the exception handler.
 */

#include <iostream>
#include <stdexcept>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// A reaction to an exception.
// Will finish SObjectizer work.
class sample_event_exception_response_action_t
	:
		public so_5::rt::event_exception_response_action_t
{
	public:
		sample_event_exception_response_action_t(
			// SObjectizer Environment to be stopped.
			so_5::rt::so_environment_t & so_environment )
			:
				m_so_environment( so_environment )
		{}

		virtual ~sample_event_exception_response_action_t()
		{}

		virtual void
		respond_to_exception()
		{
			std::cout
				<< "Respond to exception:  stop SO Environment."
				<< std::endl;

			m_so_environment.stop();
		}

	private:
		so_5::rt::so_environment_t & m_so_environment;
};

// Exception handler class.
class sample_event_exception_handler_t
	:
		public so_5::rt::event_exception_handler_t
{
	public:
		virtual ~sample_event_exception_handler_t()
		{}

		// A reaction to exception.
		virtual so_5::rt::event_exception_response_action_unique_ptr_t
		handle_exception(
			so_5::rt::so_environment_t & so_environment,
			const std::exception & ,
			const std::string &  )
		{
			return so_5::rt::event_exception_response_action_unique_ptr_t(
				new sample_event_exception_response_action_t( so_environment ) );
		}
};

// A class of an agent which will throw an exception.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t( so_5::rt::so_environment_t & env )
			: base_type_t( env )
		{}
		virtual ~a_hello_t()
		{}

		virtual void
		so_evt_start()
		{
			so_environment().install_exception_handler(
				so_5::rt::event_exception_handler_unique_ptr_t(
					new sample_event_exception_handler_t ) );

			throw std::runtime_error( "sample exception" );
		}
};

// The SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	// Creating a cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agent to the cooperation.
	coop->add_agent(
		so_5::rt::agent_ref_t(
			new a_hello_t( env ) ) );

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

