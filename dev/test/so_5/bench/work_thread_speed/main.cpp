/*
 * Working thread perfomance benchmark.
 *
 * Benchmark is performed the following way:
 * - pairs of senders-receivers are registered. They are working
 *   on active_group dispatcher;
 * - benchmarking details are set in configuration file;
 * - a processing time is measured.
 */

#include <iostream>
#include <iomanip>
#include <time.h>

#include <ace/Thread_Manager.h>

#include <so_5/rt/h/rt.hpp>

#include <so_5/disp/active_group/h/pub.hpp>

#include "a_bench_sender.hpp"
#include "a_bench_receiver.hpp"
#include "a_bench_arbiter.hpp"
#include "tag_bench_cfg.hpp"


class so_environment_t
	:
		public so_5::rt::so_environment_t
{
		typedef so_5::rt::so_environment_t base_type_t;

	public:
		so_environment_t(
			const std::string & cfg )
			:
				base_type_t(
					so_5::rt::so_environment_params_t()
					.add_named_dispatcher(
						so_5::rt::nonempty_name_t( "active_group" ),
						so_5::disp::active_group::create_disp() ) ),
				m_cfg( cfg )
		{}

		virtual ~so_environment_t(){}

		virtual void
		init()
		{
			so_5::rt::agent_coop_unique_ptr_t coop = create_coop(
				so_5::rt::nonempty_name_t( "arbiter_coop" ) );

			coop->add_agent(
				new a_bench_arbiter_t( *this, m_cfg ) );

			register_coop( std::move( coop ) );
		}

	private:
		const std::string m_cfg;
};

int main( int argc, char **argv )
{
	std::string cfg_file;
	if( 2 < argc )
	{
		std::cerr << "Wrong number of arguments\n"
			"usage: test.bench.work_thread_speed cfg_file"
			<< std::endl;
		return 1;
	}

	if( 1 == argc )
	{
		cfg_file = "./test/so_5/bench/work_thread_speed/bench.cfg";
	}
	else
		cfg_file = argv[ 1 ];

	try
	{
		so_environment_t env( cfg_file );
		env.run();

		// Wait for finish of all threads.
		ACE_Thread_Manager::instance()->wait();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		return 1;
	}


	return 0;
}
