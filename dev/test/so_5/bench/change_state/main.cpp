/*
 * A simple benchmark for so_change_state() performance.
 */

#include <iostream>
#include <iterator>
#include <numeric>
#include <chrono>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

#include <test/so_5/bench/benchmark_helpers.hpp>

struct msg_dummy : public so_5::rt::signal_t {};

class a_test_t
	:	public so_5::rt::agent_t
	{
	public :
		a_test_t(
			so_5::rt::so_environment_t & env,
			unsigned int iterations )
			:	so_5::rt::agent_t( env )
			,	st_0( self_ptr(), "0" )
			,	st_1( self_ptr(), "1" )
			,	st_2( self_ptr(), "2" )
			,	st_3( self_ptr(), "3" )
			,	st_4( self_ptr(), "4" )
			,	st_5( self_ptr(), "5" )
			,	st_6( self_ptr(), "6" )
			,	st_7( self_ptr(), "7" )
			,	st_8( self_ptr(), "8" )
			,	st_9( self_ptr(), "9" )
			,	m_iterations( iterations )
			{
				m_states.push_back( &st_0 );
				m_states.push_back( &st_1 );
				m_states.push_back( &st_2 );
				m_states.push_back( &st_3 );
				m_states.push_back( &st_4 );
				m_states.push_back( &st_5 );
				m_states.push_back( &st_6 );
				m_states.push_back( &st_7 );
				m_states.push_back( &st_8 );
				m_states.push_back( &st_9 );
			}

		virtual void
		so_define_agent()
			{
				so_subscribe( so_direct_mbox() )
						.in( st_0 )
						.in( st_1 )
						.in( st_2 )
						.in( st_3 )
						.in( st_4 )
						.in( st_5 )
						.in( st_6 )
						.in( st_7 )
						.in( st_8 )
						.in( st_9 ).event( &a_test_t::evt_dummy );
			}

		virtual void
		so_evt_start()
			{
				benchmarker_t bench;
				bench.start();

				unsigned long long changes = 0;
				for( unsigned int i = 0; i != m_iterations; ++i )
					{
						for( auto sp : m_states )
							{
								so_change_state( *sp );
								++changes;
							}
					}

				bench.finish_and_show_stats( changes, "changes" );

				so_environment().stop();
			}

		void
		evt_dummy(
			const so_5::rt::event_data_t< msg_dummy > & )
			{
			}

	private :
		const so_5::rt::state_t st_0;
		const so_5::rt::state_t st_1;
		const so_5::rt::state_t st_2;
		const so_5::rt::state_t st_3;
		const so_5::rt::state_t st_4;
		const so_5::rt::state_t st_5;
		const so_5::rt::state_t st_6;
		const so_5::rt::state_t st_7;
		const so_5::rt::state_t st_8;
		const so_5::rt::state_t st_9;

		unsigned int m_iterations;

		std::vector< const so_5::rt::state_t * > m_states;
	};

void
init(
	so_5::rt::so_environment_t & env,
	unsigned int tick_count )
	{
		env.register_agent_as_coop( "test", new a_test_t( env, tick_count ) );
	}

int
main( int argc, char ** argv )
{
	try
	{
		const unsigned int tick_count = 2 == argc ? std::atoi( argv[1] ) : 1000;

		so_5::api::run_so_environment_with_parameter( &init, tick_count );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

