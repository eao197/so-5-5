/*
 * A test of subscription to messages which cannot be null.
 */

#include <iostream>
#include <exception>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/h/types.hpp>

struct msg_test{};
struct msg_stop{};

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

		void
		evt_not_for_execution(
			const so_5::rt::not_null_event_data_t< msg_test > & )
		{
			std::cerr << "Error: evt_not_for_execution handler called..." << std::endl;
			ACE_OS::abort();
		}

		void
		evt_stop(
			const so_5::rt::event_data_t< msg_stop > & )
		{
			so_environment().stop();
		}

	private:
		// A mbox for subscription.
		so_5::rt::mbox_ref_t m_mbox;
};


void
test_agent_t::so_define_agent()
{
	so_subscribe( m_mbox )
		.event( &test_agent_t::evt_not_for_execution );
	so_subscribe( m_mbox )
		.event( &test_agent_t::evt_stop );
}

void
test_agent_t::so_evt_start()
{
	m_mbox->deliver_message< msg_test >();
	m_mbox->deliver_message< msg_stop >();
}

void
init(
	so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "test_coop" ) );

	coop->add_agent( so_5::rt::agent_ref_t( new test_agent_t( env ) ) );
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

