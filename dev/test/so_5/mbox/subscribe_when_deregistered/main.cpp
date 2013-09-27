/*
 * A test for subscription inside so_evt_finish() where they
 * should have no effects.
 */

#include <iostream>
#include <exception>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/h/types.hpp>

#include <so_5/disp/active_obj/h/pub.hpp>

struct msg1{};
struct msg2{};
struct msg3{};
struct msg4{};
struct msg5{};

class test_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		test_agent_t(
			so_5::rt::so_environment_t & env )
			:
				base_type_t( env ),
				m_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~test_agent_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

		virtual void
		so_evt_finish();

#define ABORT_HANDLER( handler, msg ) \
	void\
	handler ( const so_5::rt::event_data_t< msg > & ) \
	{\
		std::cerr << "Error: " #msg " handler called..." << std::endl; \
		ACE_OS::abort(); \
	}
		ABORT_HANDLER( handler1, msg1 )
		ABORT_HANDLER( handler2, msg2 )
		ABORT_HANDLER( handler3, msg3 )
		ABORT_HANDLER( handler4, msg4 )
		ABORT_HANDLER( handler5, msg5 )

	private:
		// Mbox for subscription.
		so_5::rt::mbox_ref_t m_mbox;
};


void
test_agent_t::so_define_agent()
{
}

// A flag which enables subscription.
so_5::atomic_counter_t g_continue_subscr_circle = 1;

void
test_agent_t::so_evt_start()
{
	try
	{
		while( 1 == g_continue_subscr_circle.value() )
		{
			so_subscribe( m_mbox )
				.event( &test_agent_t::handler1 );
			so_subscribe( m_mbox )
				.event( &test_agent_t::handler2 );
			so_subscribe( m_mbox )
				.event( &test_agent_t::handler3 );
			so_subscribe( m_mbox )
				.event( &test_agent_t::handler4 );
			so_subscribe( m_mbox )
				.event( &test_agent_t::handler5 );
		}
	}
	catch( const std::exception & )
	{}

}

void
test_agent_t::so_evt_finish()
{
	so_subscribe( m_mbox )
		.event( &test_agent_t::handler1 );
	so_subscribe( m_mbox )
		.event( &test_agent_t::handler2 );
	so_subscribe( m_mbox )
		.event( &test_agent_t::handler3 );
	so_subscribe( m_mbox )
		.event( &test_agent_t::handler4 );
	so_subscribe( m_mbox )
		.event( &test_agent_t::handler5 );
}

void
init(
	so_5::rt::so_environment_t & env )
{
	for( int i = 0; i < 8; ++i )
	{
		if( i > 0 )
			ACE_OS::sleep( ACE_Time_Value( 0, 20*1000 ) );

		g_continue_subscr_circle = 1;

		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( "test_coop" ),
			so_5::disp::active_obj::create_disp_binder( "active_obj" ) );

		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
		coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );

		env.register_coop( std::move( coop ) );
		env.deregister_coop( so_5::rt::nonempty_name_t( "test_coop" ) );
//FIXME: must not rely on timer!
		ACE_OS::sleep( ACE_Time_Value( 0, 500*1000 ) );
		g_continue_subscr_circle = 0;
	}
	env.stop();
}

int
main( int, char ** )
{
	try
	{
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
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

