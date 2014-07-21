/*
 * A benchmark for thread_pool dispatcher.
 */

#include <iostream>
#include <iterator>
#include <numeric>
#include <chrono>
#include <functional>
#include <sstream>
#include <cstdlib>

#include <ace/OS.h>
#include <ace/Get_Opt.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/disp/thread_pool/h/pub.hpp>

#include <test/so_5/bench/benchmark_helpers.hpp>

struct cfg_t
	{
		std::size_t m_cooperations = 1024;
		std::size_t m_agents = 512;
		std::size_t m_messages = 100;
		std::size_t m_demands_at_once = 0;
		std::size_t m_threads = 0;
		bool m_individual_fifo = false;
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
	ACE_Get_Opt opt( argc, argv, ":c:a:m:d:t:ih" );
	if( -1 == opt.long_option(
			"cooperations", 'c', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'cooperations'"  );
	if( -1 == opt.long_option(
			"agents", 'a', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'agents'" );
	if( -1 == opt.long_option(
			"messages", 'm', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'messages'"  );
	if( -1 == opt.long_option(
			"demands-at-once", 'd', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'demands-at-once'" );
	if( -1 == opt.long_option(
			"threads", 't', ACE_Get_Opt::ARG_REQUIRED ) )
		throw std::runtime_error( "Unable to set long option 'threads'" );
	if( -1 == opt.long_option(
			"individual-fifo", 'i', ACE_Get_Opt::NO_ARG ) )
		throw std::runtime_error( "Unable to set long option 'individual-fifo'" );
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
							"_test.bench.so_5.thread_pool_disp <options>\n"
							"\noptions:\n"
							"-c, --cooperations     count of cooperations\n"
							"-a, --agents           count of agents in cooperation\n"
							"-m, --messages         count of messages for every agent\n"
							"-d, --demands-at-once  count consequently processed demands\n"
							"-t, --threads          size of thread pool\n"
							"-i, --individual-fifo  use individual FIFO for agents\n"
							"-h, --help             show this description\n"
							<< std::endl;
					std::exit(1);
				break;

				case 'c' :
					tmp_cfg.m_cooperations = arg_to_value( opt.opt_arg() );
				break;

				case 'a' :
					tmp_cfg.m_agents = arg_to_value( opt.opt_arg() );
				break;

				case 'm' :
					tmp_cfg.m_messages = arg_to_value( opt.opt_arg() );
				break;

				case 'd' :
					tmp_cfg.m_demands_at_once = arg_to_value( opt.opt_arg() );
				break;

				case 't' :
					tmp_cfg.m_threads = arg_to_value( opt.opt_arg() );
				break;

				case 'i' :
					tmp_cfg.m_individual_fifo = true;
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

inline std::size_t
total_messages( const cfg_t & cfg )
{
	const auto total_agents = cfg.m_agents * cfg.m_cooperations;
	return total_agents + total_agents + total_agents * cfg.m_messages;
}

struct msg_start : public so_5::rt::signal_t {};
struct msg_shutdown : public so_5::rt::signal_t {};

struct msg_hello : public so_5::rt::signal_t {};

class a_test_t : public so_5::rt::agent_t
{
	public:
		a_test_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & controller_mbox,
			std::size_t messages_to_send )
			:	so_5::rt::agent_t( env )
			,	m_controller_mbox( controller_mbox )
			,	m_messages_to_send( messages_to_send )
			,	m_messages_sent( 0 )
		{
		}

		virtual void
		so_define_agent()
		{
			so_subscribe( m_controller_mbox )
					.event( so_5::signal< msg_start >, &a_test_t::evt_start );
				
			so_subscribe( so_direct_mbox() )
					.event( so_5::signal< msg_hello >, &a_test_t::evt_hello );
		}

		void
		evt_start()
		{
			so_direct_mbox()->deliver_signal< msg_hello >();
		}

		void
		evt_hello()
		{
			++m_messages_sent;
			if( m_messages_sent >= m_messages_to_send )
				m_controller_mbox->deliver_signal< msg_shutdown >();
			else
				so_direct_mbox()->deliver_signal< msg_hello >();
		}

	private :
		const so_5::rt::mbox_ref_t m_controller_mbox;

		const std::size_t m_messages_to_send;
		std::size_t m_messages_sent;
};

class a_contoller_t : public so_5::rt::agent_t
{
	public :
		a_contoller_t(
			so_5::rt::so_environment_t & env,
			cfg_t cfg )
			:	so_5::rt::agent_t( env )
			,	m_cfg( std::move( cfg ) )
			,	m_working_agents( cfg.m_cooperations * cfg.m_agents )
			,	m_self_mbox( env.create_local_mbox() )
		{}

		virtual void
		so_define_agent()
		{
			so_subscribe( m_self_mbox )
				.event( so_5::signal< msg_shutdown >,
						&a_contoller_t::evt_shutdown );
		}

		void
		so_evt_start()
		{
			create_cooperations();

			m_benchmarker.start();
			m_self_mbox->deliver_signal< msg_start >();
		}

		void
		evt_shutdown()
		{
			--m_working_agents;
			if( !m_working_agents )
			{
				m_benchmarker.finish_and_show_stats(
						total_messages( m_cfg ), "messages" );

				so_environment().stop();
			}
		}

	private :
		const cfg_t m_cfg;
		std::size_t m_working_agents;

		const so_5::rt::mbox_ref_t m_self_mbox;

		benchmarker_t m_benchmarker;

		void
		create_cooperations()
		{
			duration_meter_t duration( "creating cooperations" );

			so_5::disp::thread_pool::params_t params;
			if( m_cfg.m_individual_fifo )
				params.fifo( so_5::disp::thread_pool::fifo_t::individual );
			if( m_cfg.m_demands_at_once )
				params.max_demands_at_once( m_cfg.m_demands_at_once );

			for( std::size_t i = 0; i != m_cfg.m_cooperations; ++i )
			{
				std::ostringstream ss;
				ss << "coop_" << i;

				auto c = so_environment().create_coop( ss.str(),
						so_5::disp::thread_pool::create_disp_binder(
								"thread_pool", params ) );
				for( std::size_t a = 0; a != m_cfg.m_agents; ++a )
				{
					c->add_agent(
							new a_test_t(
									so_environment(),
									m_self_mbox,
									m_cfg.m_messages ) );
				}
				so_environment().register_coop( std::move( c ) );
			}
		}
};

std::size_t 
default_thread_pool_size()
{
	auto c = std::thread::hardware_concurrency();
	if( !c )
		c = 4;

	return c;
}

void
show_cfg( const cfg_t & cfg )
{
	std::cout << "coops: " << cfg.m_cooperations
			<< ", agents in coop: " << cfg.m_agents
			<< ", msg per agent: " << cfg.m_messages
			<< ", total msgs: " << total_messages( cfg )
			<< "\n*** demands_at_once: ";
	if( cfg.m_demands_at_once )
		std::cout << cfg.m_demands_at_once;
	else
		std::cout << "default ("
			<< so_5::disp::thread_pool::params_t().query_max_demands_at_once()
			<< ")";

	std::cout << "\n*** threads in pool: ";
	if( cfg.m_threads )
		std::cout << cfg.m_threads;
	else
		std::cout << "default ("
			<< default_thread_pool_size() << ")";

	std::cout << "\n*** FIFO: ";
	if( cfg.m_individual_fifo )
		std::cout << "individual";
	else
	{
		std::cout << "default (";
		if( so_5::disp::thread_pool::fifo_t::cooperation ==
				so_5::disp::thread_pool::params_t().query_fifo() )
			std::cout << "cooperation";
		else
			std::cout << "individual";
		std::cout << ")";
	}

	std::cout << std::endl;
}

int
main( int argc, char ** argv )
{
	try
	{
		cfg_t cfg = try_parse_cmdline( argc, argv );
		show_cfg( cfg );

		so_5::api::run_so_environment(
			[cfg]( so_5::rt::so_environment_t & env )
			{
				env.register_agent_as_coop( "test",
						new a_contoller_t( env, cfg ) );
			},
			[cfg]( so_5::rt::so_environment_params_t & params )
			{
				params.add_named_dispatcher(
					"thread_pool",
					so_5::disp::thread_pool::create_disp(
							cfg.m_threads ?
								cfg.m_threads :
								default_thread_pool_size() ) );
			});
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

