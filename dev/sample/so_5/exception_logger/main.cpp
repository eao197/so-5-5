/*
 * A sample of exception logger.
 */

#include <iostream>
#include <stdexcept>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// A class of exception logger.
class sample_event_exception_logger_t
	:
		public so_5::rt::event_exception_logger_t
{
	public:
		virtual ~sample_event_exception_logger_t()
		{}

		// A reaction to an exception.
		virtual void
		log_exception(
			const std::exception & event_exception,
			const std::string & coop_name )
		{
			std::cerr
				<< "Event_exception, coop:"
				<< coop_name << "; "
				" error: "
				<< event_exception.what()
				<< std::endl;
		}
};

// A class of agent which will throw an exception.
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

		// A reaction to start work in SObjectizer.
		virtual void
		so_evt_start()
		{
			so_environment().install_exception_logger(
				so_5::rt::event_exception_logger_unique_ptr_t(
					new sample_event_exception_logger_t ) );

			throw std::runtime_error( "sample exception" );
		}

		// A reaction to finish work in SObjectizer.
		virtual void
		so_evt_finish()
		{
			// Stopping SObjectizer.
			so_environment().stop();
		}
};

// SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	// Creating cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agent to cooperation.
	coop->add_agent(
		so_5::rt::agent_ref_t(
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
