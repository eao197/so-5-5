/*
 * A test for showing overhead of working threads in abcense of
 * any workload.
 */

#include <iostream>
#include <iterator>
#include <numeric>
#include <chrono>
#include <functional>
#include <sstream>
#include <cstdlib>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/disp/thread_pool/h/pub.hpp>
#include <so_5/disp/adv_thread_pool/h/pub.hpp>

#include <test/so_5/bench/benchmark_helpers.hpp>

enum class dispatcher_t
	{
		thread_pool,
		adv_thread_pool
	};

struct cfg_t
	{
		std::size_t m_pause = 30;
		std::size_t m_threads = 0;
		dispatcher_t m_dispatcher = dispatcher_t::thread_pool;
	};

std::size_t
arg_to_value( const char * arg )
	{
		std::stringstream ss;
		ss << arg;
		ss.seekg(0);

		std::size_t r;
		ss >> r;

		if( !ss || !ss.eof() )
			throw std::runtime_error(
					std::string( "unable to parse value: " ) + arg );

		return r;
	}

cfg_t
try_parse_cmdline(
	int argc,
	char ** argv )
{
	ACE_Get_Opt opt( argc, argv, ":d:t:p:h" );
	if( -1 == opt.long_option(
			"dispatcher", 'd', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'dispatcher'"  );
	if( -1 == opt.long_option(
			"threads", 't', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'threads'" );
	if( -1 == opt.long_option(
			"pause", 'p', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'pause'" );
	if( -1 == opt.long_option(
			"help", 'h', ACE_Get_Opt::NO_ARG ) )
		throw std::runtime_error( "Unable to set long option 'help'" );

	cfg_t tmp_cfg;

	int o;
	while( EOF != ( o = opt() ) )
		{
			switch( o )
				{
				case 'h' :
					std::cout << "usage:\n"
							"_test.bench.so_5.no_workload <options>\n"
							"\noptions:\n"
							"-d, --dispatcher  type of dispatcher\n"
							"-t, --threads     size of thread pool\n"
							"-p, --pause       timeout before exit (in seconds)\n"
							"-h, --help        show this description\n"
							<< std::endl;
					std::exit(1);
				break;

				case 'd' :
					if( std::string( "thread_pool" ) == opt.opt_arg() )
						tmp_cfg.m_dispatcher = dispatcher_t::thread_pool;
					else if( std::string( "adv_thread_pool" ) == opt.opt_arg() )
						tmp_cfg.m_dispatcher = dispatcher_t::adv_thread_pool;
					else
						throw std::runtime_error(
								std::string( "unsupported dispacther type: " ) +
								opt.opt_arg() );
				break;

				case 't' :
					tmp_cfg.m_threads = arg_to_value( opt.opt_arg() );
				break;

				case 'p' :
					tmp_cfg.m_pause = arg_to_value( opt.opt_arg() );
				break;

				case ':' :
					{
						std::ostringstream ss;
						ss << "-" << opt.opt_opt() << " requires an argument";
						throw std::runtime_error( ss.str() );
					}
				}
		}

	if( opt.opt_ind() < argc )
		throw std::runtime_error(
				std::string( "unknown argument: " ) + argv[ opt.opt_ind() ] );

	return tmp_cfg;
}

struct msg_shutdown : public so_5::rt::signal_t {};

class a_contoller_t : public so_5::rt::agent_t
{
	public :
		a_contoller_t(
			so_5::rt::so_environment_t & env,
			cfg_t cfg )
			:	so_5::rt::agent_t( env )
			,	m_cfg( std::move( cfg ) )
			,	m_remaining( m_cfg.m_pause )
		{}

		virtual void
		so_define_agent()
		{
			so_subscribe( so_direct_mbox() )
				.event( so_5::signal< msg_shutdown >,
						&a_contoller_t::evt_shutdown );
		}

		void
		so_evt_start()
		{
			m_timer = so_environment().schedule_timer< msg_shutdown >(
					so_direct_mbox(),
					1000,
					1000 );
		}

		void
		evt_shutdown()
		{
			--m_remaining;
			std::cout << "remaining: " << m_remaining << "s  \r" << std::flush;
			if( !m_remaining )
				so_environment().stop();
		}

	private :
		const cfg_t m_cfg;

		std::size_t m_remaining;

		so_5::timer_thread::timer_id_ref_t m_timer;
};

std::size_t 
default_thread_pool_size()
{
	auto c = std::thread::hardware_concurrency();
	if( !c )
		c = 4;

	return c;
}

so_5::rt::dispatcher_unique_ptr_t
create_dispatcher( const cfg_t & cfg )
{
	const auto threads = cfg.m_threads ?
			cfg.m_threads : default_thread_pool_size();

	if( dispatcher_t::adv_thread_pool == cfg.m_dispatcher )
		return so_5::disp::adv_thread_pool::create_disp( threads );

	return so_5::disp::thread_pool::create_disp( threads );
}

int
main( int argc, char ** argv )
{
	try
	{
		cfg_t cfg = try_parse_cmdline( argc, argv );

		so_5::api::run_so_environment(
			[cfg]( so_5::rt::so_environment_t & env )
			{
				env.register_agent_as_coop( "test",
						new a_contoller_t( env, cfg ) );
			},
			[cfg]( so_5::rt::so_environment_params_t & params )
			{
				params.add_named_dispatcher(
					"dispatcher",
					create_dispatcher( cfg ) );
			});
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

