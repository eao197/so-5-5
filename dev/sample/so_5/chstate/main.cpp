/*
 * A sample of the simpliest agent which has several states.
 * The agent uses different handlers for the same message.
 * In the beginning of its work agent initiates a periodic message.
 * Then agent handles this messages and switches from one state
 * to another.
 *
 * A work of the SObjectizer Environment is finished after agent
 * switched to the final state.
 *
 * State switching is fixed by a state listener.
 */

#include <iostream>
#include <time.h>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Main SObjectizer header files.
#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

// Periodic message.
class msg_periodic : public so_5::rt::signal_t {};

// State listener for fixing state changes.
class state_monitor_t
	:
		public so_5::rt::agent_state_listener_t
{
	const std::string m_type_hint;

	public:
		state_monitor_t( const std::string & type_hint )
			:
				m_type_hint( type_hint )
		{}

		virtual ~state_monitor_t()
		{}

		virtual void
		changed(
			so_5::rt::agent_t &,
			const so_5::rt::state_t & state )
		{
			std::cout << m_type_hint << " agent changed state to "
				<< state.query_name()
				<< std::endl;
		}
};

// A sample agent class.
class a_state_swither_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

		// Agent states.
		so_5::rt::state_t m_state_1;
		so_5::rt::state_t m_state_2;
		so_5::rt::state_t m_state_3;
		so_5::rt::state_t m_state_shutdown;

	public:
		a_state_swither_t( so_5::rt::so_environment_t & env )
			:
				base_type_t( env ),
				m_state_1( self_ptr(), "state_1" ),
				m_state_2( self_ptr(), "state_2" ),
				m_state_3( self_ptr(), "state_3" ),
				m_state_shutdown( self_ptr(), "state_shutdown" ),
				m_self_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~a_state_swither_t()
		{}

		// Definition of agent for SObjectizer.
		virtual void
		so_define_agent();

		// Reaction to start into SObjectizer.
		virtual void
		so_evt_start();

		// Message handler for default state.
		void
		evt_handler_default(
			const so_5::rt::event_data_t< msg_periodic > & );

		// Message handler for state_1.
		void
		evt_handler_1(
			const so_5::rt::event_data_t< msg_periodic > & );

		// Message handler for state_2.
		void
		evt_handler_2(
			const so_5::rt::event_data_t< msg_periodic > & );

		// Message handler for state_3.
		void
		evt_handler_3(
			const so_5::rt::event_data_t< msg_periodic > & );

		// Message handler for shutdown_state.
		void
		evt_handler_shutdown(
			const so_5::rt::event_data_t< msg_periodic > & );

	private:
		// Mbox for that agent.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Timer event id.
		// If we do not store it the periodic message will
		// be canceled automatically.
		so_5::timer_thread::timer_id_ref_t m_timer_id;
};

void
a_state_swither_t::so_define_agent()
{
	// Subsription to message.
	so_subscribe( m_self_mbox )
		.event( &a_state_swither_t::evt_handler_default );

	so_subscribe( m_self_mbox )
		.in( m_state_1 )
		.event( &a_state_swither_t::evt_handler_1 );

	so_subscribe( m_self_mbox )
		.in( m_state_2 )
		.event( &a_state_swither_t::evt_handler_2 );

	so_subscribe( m_self_mbox )
		.in( m_state_3 )
		.event( &a_state_swither_t::evt_handler_3 );

	so_subscribe( m_self_mbox )
		.in( m_state_shutdown )
		.event( &a_state_swither_t::evt_handler_shutdown );
}

void
a_state_swither_t::so_evt_start()
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "a_state_swither_t::so_evt_start()" << std::endl;

	// Periodic message should be initiated.
	m_timer_id = so_environment().schedule_timer< msg_periodic >(
			m_self_mbox,
			1 * 1000,
			1 * 1000 );
}

void
a_state_swither_t::evt_handler_default(
	const so_5::rt::event_data_t< msg_periodic > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "evt_handler_default" << std::endl;

	// Switching to next state.
	so_change_state( m_state_1 );
}

void
a_state_swither_t::evt_handler_1(
	const so_5::rt::event_data_t< msg_periodic > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "evt_handler_1, state: " << so_current_state().query_name()
		<< std::endl;

	// Switching to next state.
	so_change_state( m_state_2 );
}

void
a_state_swither_t::evt_handler_2(
	const so_5::rt::event_data_t< msg_periodic > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "evt_handler_2, state: " << so_current_state().query_name()
		<< std::endl;

	// Switching to next state.
	so_change_state( m_state_3 );
}

void
a_state_swither_t::evt_handler_3(
	const so_5::rt::event_data_t< msg_periodic > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "evt_handler_3, state: " << so_current_state().query_name()
		<< std::endl;

	// Switching to next state.
	so_change_state( m_state_shutdown );
}

void
a_state_swither_t::evt_handler_shutdown(
	const so_5::rt::event_data_t< msg_periodic > & )
{
	time_t t = time( 0 );
	std::cout << asctime( localtime( &t ) )
		<< "evt_handler_shutdown, state: "
		<< so_current_state().query_name()
		<< std::endl;

	// Switching to the default state.
	so_change_state( so_default_state() );

	// Finishing SObjectizer work.
	std::cout << "Stop sobjectizer..." << std::endl;
	so_environment().stop();
}

// State listener.
state_monitor_t g_state_monitor( "nondestroyable_listener" );

// SObjectizer Environment initialization.
void
init( so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_ref_t ag( new a_state_swither_t( env ) );

	// Adding state listener. Its life-time is not controlled by agent.
	ag->so_add_nondestroyable_listener( g_state_monitor );

	// Adding another state listener.
	// Its life-time is controlled by agent.
	ag->so_add_destroyable_listener(
		so_5::rt::agent_state_listener_unique_ptr_t(
			new state_monitor_t( "destroyable_listener" ) ) );

	// Creating cooperation.
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Adding agent to cooperation.
	coop->add_agent( ag );

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
