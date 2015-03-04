/*
 * A simple test for message limits (redirecting service request).
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct msg_request : public so_5::rt::signal_t {};

class a_worker_t : public so_5::rt::agent_t
{
public :
	a_worker_t(
		so_5::rt::environment_t & env,
		std::string reply )
		:	so_5::rt::agent_t( env,
				tuning_options().message_limits(
					message_limit< msg_request >( 1 ).drop ) )
		,	m_reply( std::move( reply ) )
	{}

	a_worker_t(
		so_5::rt::environment_t & env,
		std::string reply,
		const so_5::rt::mbox_t & redirect_to )
		:	so_5::rt::agent_t( env,
				tuning_options().message_limits(
					message_limit< msg_request >( 1 ).redirect( redirect_to ) ) )
		,	m_reply( std::move( reply ) )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event< msg_request >(
			so_direct_mbox(), [&]() -> std::string { return m_reply; } );
	}

private :
	const std::string m_reply;
};

class a_manager_t : public so_5::rt::agent_t
{
public :
	a_manager_t(
		so_5::rt::environment_t & env,
		so_5::rt::mbox_t target_mbox,
		std::string expected_response )
		:	so_5::rt::agent_t( env )
		,	m_target_mbox( std::move( target_mbox ) )
		,	m_expected_response( expected_response )
	{}

	virtual void
	so_evt_start() override
	{
		auto svc = m_target_mbox->get_one< std::string >().wait_forever();

		m_responses += svc.sync_get< msg_request >();
		m_responses += svc.sync_get< msg_request >();
		m_responses += svc.sync_get< msg_request >();

		if( m_expected_response == m_responses )
			so_deregister_agent_coop_normally();
	}

private :
	const so_5::rt::mbox_t m_target_mbox;
	const std::string m_expected_response;
	std::string m_responses;
};

void
init( so_5::rt::environment_t & env )
{
	auto coop = env.create_coop( so_5::autoname );
	
	auto worker_disp = so_5::disp::one_thread::create_private_disp();

	auto w3 = coop->make_agent_with_binder< a_worker_t >(
			worker_disp->binder(),
			"[three]" );
	auto w2 = coop->make_agent_with_binder< a_worker_t >(
			worker_disp->binder(), 
			"[two]", w3->so_direct_mbox() );
	auto w1 = coop->make_agent_with_binder< a_worker_t >(
			worker_disp->binder(),
			"[one]", w2->so_direct_mbox() );

	coop->make_agent< a_manager_t >( w1->so_direct_mbox(), "[one][two][three]" );

	env.register_coop( std::move( coop ) );
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
			2,
			"simple service request redirect on MPMC-mboxes test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

