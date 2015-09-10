/*
 * A simple test for v.5.5.9 helper functions for synchronous
 * interactions.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct classic_msg : public so_5::rt::message_t
{
	std::string m_a;
	std::string m_b;

	classic_msg( std::string a, std::string b )
		:	m_a( std::move(a) ), m_b( std::move(b) )
		{}
};

struct msg
{
	std::string m_a;
	std::string m_b;
};

struct empty {};

struct classic_signal : public so_5::rt::signal_t {};

template< typename TARGET >
void
setup_service_events( TARGET & to )
{
	to
		.event( []( int evt ) -> std::string {
				return "i{" + std::to_string( evt ) + "}";
			} )
		.event( []( const classic_msg & evt ) {
				return "cm{" + evt.m_a + "," + evt.m_b + "}";
			} )
		.event( []( const msg & evt ) -> std::string {
				return "m{" + evt.m_a + "," + evt.m_b + "}";
			} )
		.event( []( empty ) -> std::string {
				return "empty{}";
			} )
		.template event< classic_signal >( []() -> std::string {
				return "signal{}";
			} );
}

template< typename TARGET >
void
perform_service_interaction( TARGET && service )
{
	std::string accumulator;

	accumulator += so_5::make_async_get< std::string, int >( service, 1 ).get();

	accumulator += so_5::make_async_get< std::string, classic_msg >(
			service, "Hello", "World" ).get();

	accumulator += so_5::make_async_get< std::string, msg >(
			service, "Bye", "World" ).get();

	accumulator += so_5::make_async_get< std::string, classic_signal >(
			service ).get();

	accumulator += so_5::make_async_get< std::string, empty >(
			service ).get();

	const std::string expected = "i{1}cm{Hello,World}m{Bye,World}"
			"signal{}empty{}";

	if( expected != accumulator )
		throw std::runtime_error( "unexpected accumulator value: " +
				accumulator + ", expected: " + expected );
}

class a_service_t : public so_5::rt::agent_t
{
public :
	a_service_t( context_t ctx )
		:	so_5::rt::agent_t( ctx )
	{}

	virtual void
	so_define_agent() override
	{
		setup_service_events( so_default_state() );
	}
};

class a_test_via_mbox_t : public so_5::rt::agent_t
{
public :
	a_test_via_mbox_t( context_t ctx, so_5::rt::mbox_t service )
		:	so_5::rt::agent_t( ctx )
		,	m_service( std::move( service ) )
	{}

	virtual void
	so_evt_start() override
	{
		std::string accumulator;

		perform_service_interaction( m_service );

		so_deregister_agent_coop_normally();
	}

private :
	const so_5::rt::mbox_t m_service;
};

class a_test_via_direct_mbox_t : public so_5::rt::agent_t
{
public :
	a_test_via_direct_mbox_t( context_t ctx, const a_service_t & service )
		:	so_5::rt::agent_t( ctx )
		,	m_service( service )
	{}

	virtual void
	so_evt_start() override
	{
		std::string accumulator;

		perform_service_interaction( m_service );

		so_deregister_agent_coop_normally();
	}

private :
	const a_service_t & m_service;
};

void
make_adhoc_agents_coop( so_5::rt::environment_t & env )
{
	env.introduce_coop( []( so_5::rt::agent_coop_t & coop ) {
		using namespace so_5::disp::one_thread;

		auto service = coop.define_agent();
		setup_service_events( service );

		coop.define_agent().on_start( [&coop, service] {
			} );
	} );
}

void
init( so_5::rt::environment_t & env )
{
	env.introduce_coop( []( so_5::rt::agent_coop_t & coop ) {
			using namespace so_5::disp::one_thread;

			auto service = coop.make_agent_with_binder< a_service_t >(
					create_private_disp( coop.environment() )->binder() );

			coop.make_agent< a_test_via_mbox_t >( service->so_direct_mbox() );
		} );

	env.introduce_coop( []( so_5::rt::agent_coop_t & coop ) {
			using namespace so_5::disp::one_thread;

			auto service = coop.make_agent_with_binder< a_service_t >(
					create_private_disp( coop.environment() )->binder() );

			coop.make_agent< a_test_via_direct_mbox_t >( *service );
		} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( &init );
			},
			4,
			"simple user message type service_request test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

