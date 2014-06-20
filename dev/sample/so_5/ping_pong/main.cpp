#include <iostream>
#include <set>

#include <cstdio>

#include <ace/ACE.h>
#include <ace/Get_Opt.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include <so_5/api/h/api.hpp>
#include <so_5/rt/h/rt.hpp>

#include <so_5/disp/active_obj/h/pub.hpp>

struct	cfg_t
{
	unsigned int	m_request_count;

	bool	m_active_objects;

	cfg_t()
		:	m_request_count( 1000 )
		,	m_active_objects( false )
		{}
};

int
try_parse_cmdline(
	int argc,
	char ** argv,
	cfg_t & cfg )
{
	if( 1 == argc )
		{
			std::cout << "usage:\n"
					"sample.so_5.ping_pong <options>\n"
					"\noptions:\n"
					"-a, --active-objects agents should be active objects\n"
					"-r, --requests       count of requests to send\n"
					<< std::endl;

			ACE_ERROR_RETURN(
				( LM_ERROR, ACE_TEXT( "No arguments supplied\n" ) ), -1 );
		}

	ACE_Get_Opt opt( argc, argv, ":ar:" );
	if( -1 == opt.long_option(
			"active-objects", 'a', ACE_Get_Opt::NO_ARG ) )
		ACE_ERROR_RETURN(( LM_ERROR, ACE_TEXT(
						"Unable to set long option 'active-objects'\n" )), -1 );
	if( -1 == opt.long_option(
			"requests", 'r', ACE_Get_Opt::ARG_REQUIRED ) )
		ACE_ERROR_RETURN(( LM_ERROR, ACE_TEXT(
						"Unable to set long option 'requests'\n" )), -1 );

	cfg_t tmp_cfg;

	int o;
	while( EOF != ( o = opt() ) )
		{
			switch( o )
				{
				case 'a' :
					tmp_cfg.m_active_objects = true;
				break;

				case 'r' :
					tmp_cfg.m_request_count = ACE_OS::atoi( opt.opt_arg() );
				break;

				case ':' :
					ACE_ERROR_RETURN(( LM_ERROR,
							ACE_TEXT( "-%c requieres argument\n" ),
									opt.opt_opt() ), -1 );
				}
		}

	if( opt.opt_ind() < argc )
		ACE_ERROR_RETURN(( LM_ERROR,
				ACE_TEXT( "Unknown argument: '%s'\n" ),
						argv[ opt.opt_ind() ] ),
				-1 );

	cfg = tmp_cfg;

	return 0;
}

void
show_cfg(
	const cfg_t & cfg )
	{
		std::cout << "Configuration: "
			<< "active objects: " << ( cfg.m_active_objects ? "yes" : "no" )
			<< ", requests: " << cfg.m_request_count
			<< std::endl;
	}

void
run_sample(
	const cfg_t & cfg )
	{
		// This variable will be a part of pinger agent's state.
		unsigned int pings_left = cfg.m_request_count;

		so_5::api::run_so_environment(
			[&pings_left, &cfg]( so_5::rt::so_environment_t & env )
			{
				// Types of signals for the agents.
				struct msg_ping : public so_5::rt::signal_t {};
				struct msg_pong : public so_5::rt::signal_t {};

				auto mbox = env.create_local_mbox();

				auto coop = env.create_coop( "ping_pong",
					// Agents will be active or passive.
					// It depends on sample arguments.
					cfg.m_active_objects ?
						so_5::disp::active_obj::create_disp_binder( "active_obj" ) :
						so_5::rt::create_default_disp_binder() );

				// Pinger agent.
				coop->define_agent()
					.on_start( [mbox]() { mbox->deliver_signal< msg_ping >(); } )
					.event( mbox, so_5::signal< msg_pong >,
						[&pings_left, &env, mbox]()
						{
							if( pings_left ) --pings_left;
							if( pings_left )
								mbox->deliver_signal< msg_ping >();
							else
								env.stop();
						} );

				// Ponger agent.
				coop->define_agent()
					.event( mbox, so_5::signal< msg_ping >,
						[mbox]() { mbox->deliver_signal< msg_pong >(); } );

				env.register_coop( std::move( coop ) );
			},
			[&cfg]( so_5::rt::so_environment_params_t & p )
			{
				if( cfg.m_active_objects )
					// Special dispatcher is necessary for agents.
					p.add_named_dispatcher( "active_obj",
						so_5::disp::active_obj::create_disp() );
			} );
	}

int
main( int argc, char ** argv )
{
	cfg_t cfg;
	if( -1 != try_parse_cmdline( argc, argv, cfg ) )
	{
		show_cfg( cfg );

		try
		{
			run_sample( cfg );

			return 0;
		}
		catch( const std::exception & x )
		{
			std::cerr << "*** Exception caught: " << x.what() << std::endl;
		}
	}

	return 2;
}

