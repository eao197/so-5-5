/*
 * A test of unsubscription of messages in different states.
 */

#include <iostream>
#include <exception>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>


struct msg1{};
struct msg2{};
struct msg3{};
struct msg4{};
struct msg5{};

struct msg_switch_state{};
struct msg_stop{};

class test_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

		so_5::rt::state_t m_state_1;
		so_5::rt::state_t m_state_2;
		so_5::rt::state_t m_state_3;
		so_5::rt::state_t m_state_4;
		so_5::rt::state_t m_state_shutdown;
	public:
		test_agent_t(
			so_5::rt::so_environment_t & env )
			:
				base_type_t( env ),
				m_state_1( self_ptr(), "state_1" ),
				m_state_2( self_ptr(), "state_2"  ),
				m_state_3( self_ptr(), "state_3" ),
				m_state_4( self_ptr(), "state_4" ),
				m_state_shutdown( self_ptr(), "state_shutdown" )
		{}

		virtual ~test_agent_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

#define ABORT_HANDLER( handler, msg ) \
	void\
	handler ( const so_5::rt::event_data_t< msg > & ) \
	{\
		std::cerr << "Error: " #msg " handler called" \
			"in state " << so_current_state().query_name() << "..." \
			<< std::endl; \
		ACE_OS::abort(); \
	}
		ABORT_HANDLER( handler1, msg1 )
		ABORT_HANDLER( handler2, msg2 )
		ABORT_HANDLER( handler3, msg3 )
		ABORT_HANDLER( handler4, msg4 )
		ABORT_HANDLER( handler5, msg5 )

		void
		shift_state()
		{
			if( so_current_state() == so_default_state() )
				so_change_state( m_state_1 );
			else if( so_current_state() == m_state_1 )
				so_change_state( m_state_2 );
			else if( so_current_state() == m_state_2 )
				so_change_state( m_state_3 );
			else if( so_current_state() == m_state_3 )
				so_change_state( m_state_4 );
			else if( so_current_state() == m_state_4 )
				so_change_state( m_state_shutdown );
			else if( !(so_current_state() == m_state_shutdown ) )
			{
				std::cerr << "Error: non shiftable state: "
					<< so_current_state().query_name() << std::endl;
				ACE_OS::abort();
			}
		}

		void
		iteration()
		{
			so_5::rt::nonempty_name_t mbox_name( "test_mbox" );

			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::handler1 );
			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::handler2 );
			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::handler3 );
			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::handler4 );
			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::handler5 );

			so_environment().create_local_mbox( mbox_name )->deliver_message< msg1 >();
			so_environment().create_local_mbox( mbox_name )->deliver_message< msg2 >();
			so_environment().create_local_mbox( mbox_name )->deliver_message< msg3 >();
			so_environment().create_local_mbox( mbox_name )->deliver_message< msg4 >();
			so_environment().create_local_mbox( mbox_name )->deliver_message< msg5 >();

			so_environment().create_local_mbox( mbox_name )->deliver_message< msg_stop >();
			so_environment().create_local_mbox( mbox_name )->deliver_message< msg_switch_state >();
		}

		void
		evt_switch_state(
			const so_5::rt::event_data_t< msg_switch_state > & )
		{
			if( so_current_state() == m_state_shutdown )
				return;

			so_5::rt::nonempty_name_t mbox_name( "test_mbox" );

			so_unsubscribe( so_environment().create_local_mbox( mbox_name ) )
				.in( so_current_state() )
				.event( &test_agent_t::evt_switch_state );

			shift_state();
			iteration();
		}

		void
		evt_stop(
			const so_5::rt::event_data_t< msg_stop > &
				msg )
		{
			so_environment().stop();
		}
};

void
test_agent_t::so_define_agent()
{
	so_5::rt::nonempty_name_t mbox_name( "test_mbox" );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::handler5 );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::handler1 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::handler2 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::handler3 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::handler4 );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::handler5 );


	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.event( &test_agent_t::evt_switch_state );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_1 )
		.event( &test_agent_t::evt_switch_state );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_2 )
		.event( &test_agent_t::evt_switch_state );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_3 )
		.event( &test_agent_t::evt_switch_state );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_4 )
		.event( &test_agent_t::evt_switch_state );
	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::evt_switch_state );

	so_subscribe( so_environment().create_local_mbox( mbox_name ) )
		.in( m_state_shutdown )
		.event( &test_agent_t::evt_stop );
}

void
test_agent_t::so_evt_start()
{
	iteration();
}

void
init(
	so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "test_coop" ) );

	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );

	env.register_coop(
		std::move( coop ),
		so_5::THROW_ON_ERROR );
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



