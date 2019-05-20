#include <iostream>
#include <set>
#include <chrono>

#include <cstdio>
#include <cstdlib>

#include <so_5/all.hpp>

#include <various_helpers_1/cmd_line_args_helpers.hpp>
#include <various_helpers_1/benchmark_helpers.hpp>

using namespace std::chrono;

struct	cfg_t
{
	unsigned int m_root_count = 2;
	unsigned int m_levels = 5;
	unsigned int m_level_size = 5;
};

cfg_t
try_parse_cmdline(
	int argc,
	char ** argv )
{
	cfg_t tmp_cfg;

	for( char ** current = &argv[ 1 ], **last = argv + argc;
			current != last;
			++current )
		{
			if( is_arg( *current, "-h", "--help" ) )
				{
					std::cout << "usage:\n"
							"_test.bench.so_5.parallel_parent_child <options>\n"
							"\noptions:\n"
							"-r, --root-count     count of roots (parallel parents)\n"
							"-l, --levels         count of levels\n"
							"-s, --level-size     count of coop on each level\n"
							"-h, --help           show this help"
							<< std::endl;
					std::exit( 1 );
				}
			else if( is_arg( *current, "-r", "--root-count" ) )
				mandatory_arg_to_value(
						tmp_cfg.m_root_count, ++current, last,
						"-r", "count of roots (parallel parents)" );
			else if( is_arg( *current, "-l", "--levels" ) )
				mandatory_arg_to_value(
						tmp_cfg.m_levels, ++current, last,
						"-l", "count of levels" );
			else if( is_arg( *current, "-s", "--level-size" ) )
				mandatory_arg_to_value(
						tmp_cfg.m_level_size, ++current, last,
						"-s", "level size" );
			else
				throw std::runtime_error(
						std::string( "unknown argument: " ) + *current );
		}

	return tmp_cfg;
}

using disp_handle_t = so_5::disp::one_thread::private_dispatcher_handle_t;

void
create_and_register_agent(
	so_5::environment_t & env,
	const std::string & parent_coop_name,
	const so_5::mbox_t & parent_mbox,
	const disp_handle_t & disp,
	unsigned int current_level,
	unsigned int ordinal,
	unsigned int total_levels,
	unsigned int level_size );

struct msg_child_completed final {
	unsigned int m_children_agents;
};

class a_child_t final : public so_5::agent_t
{
	public :
		a_child_t(
			context_t ctx,
			so_5::mbox_t parent_mbox,
			disp_handle_t disp,
			unsigned int current_level,
			unsigned int total_levels,
			unsigned int level_size )
			:	so_5::agent_t{ std::move(ctx) }
			,	m_parent_mbox{ std::move(parent_mbox) }
			,	m_disp{ std::move(disp) }
			,	m_current_level{ current_level }
			,	m_total_levels{ total_levels }
			,	m_level_size{ level_size }
		{}

		void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_child_t::evt_child_completed );
		}

		void
		so_evt_start() override
		{
			if( m_current_level != m_total_levels )
				for( unsigned int i = 0; i != m_level_size; ++i )
					create_and_register_agent(
							so_environment(),
							so_coop_name(),
							so_direct_mbox(),
							m_disp,
							m_current_level + 1u,
							i,
							m_total_levels,
							m_level_size );
			else
				complete_work();
		}

		void
		evt_child_completed(
			mhood_t< msg_child_completed > cmd )
		{
			m_children_agents += cmd->m_children_agents + 1u;
			++m_children_completed;
			if( m_children_completed == m_level_size )
				complete_work();
		}

	private :
		const so_5::mbox_t m_parent_mbox;
		const disp_handle_t m_disp;

		const unsigned int m_current_level;
		const unsigned int m_total_levels;
		const unsigned int m_level_size;

		unsigned int m_children_agents{};
		unsigned int m_children_completed{};

		void
		complete_work()
		{
			so_5::send< msg_child_completed >( m_parent_mbox, m_children_agents );
		}
};

void
create_and_register_agent(
	so_5::environment_t & env,
	const std::string & parent_coop_name,
	const so_5::mbox_t & parent_mbox,
	const disp_handle_t & disp,
	unsigned int current_level,
	unsigned int ordinal,
	unsigned int total_levels,
	unsigned int level_size )
{
	std::ostringstream ss;
	ss << parent_coop_name << "-" << current_level << ":" << ordinal;

	env.introduce_coop( ss.str(), disp->binder(),
		[&]( so_5::coop_t & coop ) {
			coop.set_parent_coop_name( parent_coop_name );
			coop.make_agent< a_child_t >(
					parent_mbox,
					disp,
					current_level,
					total_levels,
					level_size );
		} );
}

class a_root_t final : public so_5::agent_t
{
	public :
		a_root_t(
			context_t ctx,
			disp_handle_t disp,
			std::atomic_uint & result_receiver,
			unsigned int total_levels,
			unsigned int level_size )
			:	so_5::agent_t{ std::move(ctx) }
			,	m_disp{ std::move(disp) }
			,	m_result_receiver{ result_receiver }
			,	m_total_levels{ total_levels }
			,	m_level_size{ level_size }
		{}

		void
		so_define_agent() override
		{
			so_subscribe_self()
				.event( &a_root_t::on_child_completed );
		}

		void
		so_evt_start() override
		{
			for( unsigned int i = 0; i != m_level_size; ++i )
				create_and_register_agent(
						so_environment(),
						so_coop_name(),
						so_direct_mbox(),
						m_disp,
						0u,
						i,
						m_total_levels,
						m_level_size );
		}

	private :
		const disp_handle_t m_disp;

		std::atomic_uint & m_result_receiver;

		const unsigned int m_total_levels;
		const unsigned int m_level_size;

		unsigned int m_children_agents{};
		unsigned int m_children_completed{};

		void
		on_child_completed( mhood_t<msg_child_completed> cmd )
		{
			m_children_agents += cmd->m_children_agents + 1u;
			++m_children_completed;
			if( m_children_completed == m_level_size )
			{
				m_result_receiver = m_children_agents;
				so_deregister_agent_coop_normally();
			}
		}
};

void
show_cfg(
	const cfg_t & cfg )
{
	std::cout << "Configuration: "
		<< "roots: " << cfg.m_root_count
		<< ", levels: " << cfg.m_levels
		<< ", level-size: " << cfg.m_level_size
		<< std::endl;
}

void
run_sobjectizer( const cfg_t & cfg )
{
	std::vector< std::atomic_uint > results( cfg.m_root_count );
	for( auto & v : results )
		v = 0u;

	{
		duration_meter_t meter{ "parallel_parent_child" };

		so_5::launch( [&]( so_5::environment_t & env ) {
				for( unsigned int i = 0; i != cfg.m_root_count; ++i )
				{
					auto disp = so_5::disp::one_thread::create_private_disp( env );
					env.introduce_coop(
							disp->binder(),
							[&]( so_5::coop_t & coop ) {
								coop.make_agent< a_root_t >(
										disp,
										std::ref(results[i]),
										cfg.m_levels,
										cfg.m_level_size );
							} );
				}
			} );
	}

	unsigned int total{};
	for( auto & v : results )
	{
		const auto c = v.load();
		std::cout << c << std::endl;

		total += c;
	}

	std::cout << "Total: " << total << std::endl;
}

int
main( int argc, char ** argv )
{
	try
	{
		cfg_t cfg = try_parse_cmdline( argc, argv );
		show_cfg( cfg );

		run_sobjectizer( cfg );

		return 0;
	}
	catch( const std::exception & x )
	{
		std::cerr << "*** Exception caught: " << x.what() << std::endl;
	}

	return 2;
}

