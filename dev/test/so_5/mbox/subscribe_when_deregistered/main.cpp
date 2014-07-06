/*
 * A test for subscription inside so_evt_finish() where they
 * should have no effects.
 */

#include <iostream>
#include <exception>

#include <ace/OS.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

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
		so_evt_finish();

#define ABORT_HANDLER( handler, msg ) \
	void\
	handler ( const so_5::rt::event_data_t< msg > & ) \
	{\
		std::cerr << "Error: " #msg " handler called..." << std::endl; \
		std::abort(); \
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

class stage_monitors_t
{
	private :
		ACE_Thread_Mutex m_reg_mutex;
		ACE_Condition< ACE_Thread_Mutex > m_reg_signal;

		ACE_Thread_Mutex m_dereg_mutex;
		ACE_Condition< ACE_Thread_Mutex > m_dereg_signal;

		enum stage_t {
			NOT_STARTED,
			COOP_REGISTERED,
			COOP_DEREGISTERED
		};

		stage_t m_stage;

	public :
		stage_monitors_t()
			:	m_reg_signal( m_reg_mutex )
			,	m_dereg_signal( m_dereg_mutex )
			,	m_stage( NOT_STARTED )
		{}

		void
		wait_for_registration()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_reg_mutex );
			if( COOP_REGISTERED != m_stage )
				m_reg_signal.wait();
		}

		void
		notify_about_registration()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_reg_mutex );
			m_stage = COOP_REGISTERED;
			m_reg_signal.signal();
		}

		void
		wait_for_deregistration()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_dereg_mutex );
			if( COOP_DEREGISTERED != m_stage )
				m_dereg_signal.wait();
		}

		void
		notify_about_deregistration()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_dereg_mutex );
			m_stage = COOP_DEREGISTERED;
			m_dereg_signal.signal();
		}
};

stage_monitors_t g_stage_monitors;

void
init(
	so_5::rt::so_environment_t & env )
{
	for( int i = 0; i < 8; ++i )
	{
		so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			so_5::rt::nonempty_name_t( "test_coop" ),
			so_5::disp::active_obj::create_disp_binder( "active_obj" ) );

		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );
		coop->add_agent( new test_agent_t( env ) );

		env.register_coop( std::move( coop ) );
		g_stage_monitors.wait_for_registration();

		env.deregister_coop(
				"test_coop",
				so_5::rt::dereg_reason::normal );
		g_stage_monitors.wait_for_deregistration();
	}
	env.stop();
}

class listener_t : public so_5::rt::coop_listener_t
{
	public :
		virtual void
		on_registered(
			so_5::rt::so_environment_t & so_env,
			const std::string & coop_name )
		{
			g_stage_monitors.notify_about_registration();
		}

		virtual void
		on_deregistered(
			so_5::rt::so_environment_t & so_env,
			const std::string & coop_name,
			const so_5::rt::coop_dereg_reason_t &)
		{
			g_stage_monitors.notify_about_deregistration();
		}
};

int
main( int, char ** )
{
	try
	{
		so_5::api::run_so_environment(
			&init,
			std::move(
				so_5::rt::so_environment_params_t()
					.add_named_dispatcher(
						so_5::rt::nonempty_name_t( "active_obj" ),
						so_5::disp::active_obj::create_disp() )
					.coop_listener(
						so_5::rt::coop_listener_unique_ptr_t( new listener_t() ) ) ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

