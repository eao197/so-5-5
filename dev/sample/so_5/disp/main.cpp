/*
 * A sample of working with dispatchers.
 */

#include <iostream>
#include <sstream>

#include <ace/OS.h>
#include <ace/Log_Msg.h>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// SObjectizer dispatchers.
#include <so_5/disp/one_thread/h/pub.hpp>
#include <so_5/disp/active_group/h/pub.hpp>
#include <so_5/disp/active_obj/h/pub.hpp>

// A class for an agent.
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

		// A reaction to start of work in SObjectizer.
		virtual void
		so_evt_start();

		// A reaction to finish of work in SObjectizer.
		virtual void
		so_evt_finish();

	private:
		const std::string m_name;
};

// A helper macro for the ACE Logging.
#define AGENT_MSG( s ) "TID:%t %T " s

void
a_disp_user_t::so_evt_start()
{
	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_start(): start pause\n" ), m_name.c_str() ));

	// Sleeping for some time.
	ACE_OS::sleep( 1 );

	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_start(): finish pause\n" ), m_name.c_str() ));
}

void
a_disp_user_t::so_evt_finish()
{
	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_finish(): start pause\n" ), m_name.c_str() ));

	// Sleeping for some time.
	ACE_OS::sleep( 1 );

	ACE_DEBUG(( LM_INFO,
		AGENT_MSG( "%s.so_evt_finish(): finish pause\n" ), m_name.c_str() ));
}

// The helper function for making name of an agent.
std::string
create_agent_name( const std::string & base, int i )
{
	std::ostringstream sout;
	sout << base << "_" << i;

	return sout.str();
}


// The SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	// Creating a cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agents which will work on the default dispatcher.
	for( int i = 0; i < 4; ++i )
	{
		const std::string name = create_agent_name( "default_disp", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t(
				new a_disp_user_t( env, name ) ) );
	}

	// Adding agents which will work on the dispatcher 
	// with name 'single_thread'.
	for( int i = 0; i < 3; ++i )
	{
		const std::string name = create_agent_name( "single_thread", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::one_thread::create_disp_binder(
				"single_thread" ) );
	}

	// Adding agents which will work on the dispatcher with active groups
	// named as 'active_group'. Agents will be bound to a group 'A'.
	for( int i = 0; i < 2; ++i )
	{
		const std::string name = create_agent_name( "active_group_A", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_group::create_disp_binder(
				"active_group",
				"A" ) );
	}

	// Adding agents which will work on the dispatcher with active groups
	// named as 'active_group'. Agents will be bound to a group 'B'.
	for( int i = 0; i < 2; ++i )
	{
		const std::string name = create_agent_name( "active_group_B", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_group::create_disp_binder(
				"active_group",
				"B" ) );
	}

	// Adding agents which will work on the dispatcher for active objects.
	// This dispatcher will have name 'active_obj'.
	for( int i = 0; i < 4; ++i )
	{
		const std::string name = create_agent_name( "active_obj", i+1 );

		coop->add_agent(
			so_5::rt::agent_ref_t( new a_disp_user_t( env, name ) ),
			so_5::disp::active_obj::create_disp_binder(
				"active_obj" ) );
	}

	// Registering the cooperation.
	env.register_coop( std::move( coop ) );

	// Stopping SObjectizer.
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
