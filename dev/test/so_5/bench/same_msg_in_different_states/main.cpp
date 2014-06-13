/*
 */

#include <iostream>
#include <iterator>
#include <numeric>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

struct msg_tick : public so_5::rt::signal_t {};

class a_test_t
	:	public so_5::rt::agent_t
	{
	public :
		a_test_t(
			so_5::rt::so_environment_t & env,
			int tick_count )
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
			,	m_self_mbox( env.create_local_mbox() )
			,	m_tick_count( tick_count )
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

				m_it_current_state = m_states.begin();
			}

		virtual void
		so_define_agent()
			{
				so_subscribe( m_self_mbox )
						.in( st_0 )
						.in( st_1 )
						.in( st_2 )
						.in( st_3 )
						.in( st_4 )
						.in( st_5 )
						.in( st_6 )
						.in( st_7 )
						.in( st_8 )
						.in( st_9 ).event( &a_test_t::evt_tick );
			}

		virtual void
		so_evt_start()
			{
				so_change_state( st_0 );

				m_self_mbox->deliver_signal< msg_tick >();
			}

		void
		evt_tick(
			const so_5::rt::event_data_t< msg_tick > & )
			{
				++m_it_current_state;
				if( m_it_current_state == m_states.end() )
				{
					--m_tick_count;
					m_it_current_state = m_states.begin();
				}

				if( m_tick_count > 0 )
				{
					so_change_state( **m_it_current_state );
					m_self_mbox->deliver_signal< msg_tick >();
				}
				else
					so_environment().stop();
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

		const so_5::rt::mbox_ref_t m_self_mbox;

		int m_tick_count;

		std::vector< const so_5::rt::state_t * > m_states;
		std::vector< const so_5::rt::state_t * >::iterator m_it_current_state;
	};

void
init(
	so_5::rt::so_environment_t & env,
	int tick_count )
	{
		env.register_agent_as_coop( "test", new a_test_t( env, tick_count ) );
	}

int
main( int argc, char ** argv )
{
	try
	{
		const int tick_count = 2 == argc ? std::atoi( argv[1] ) : 10;

		so_5::api::run_so_environment_with_parameter( &init, tick_count );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

