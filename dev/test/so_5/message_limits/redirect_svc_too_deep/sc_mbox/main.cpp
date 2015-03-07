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
					limit_then_redirect< msg_request >( 2,
						[this] { return so_direct_mbox(); } ) ) )
		,	m_reply( std::move( reply ) )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event< msg_request >(
			[&]() -> std::string { return m_reply; } );
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
	so_define_agent() override
	{
		so_default_state().event< msg_get_results >(
			[&] {
				std::string responses = m_r1.get() + m_r2.get();
				
				try
				{
					responses += m_r3.get();
					throw std::runtime_error( "Exception from SObjectizer expected!" );
				}
				catch( const so_5::exception_t & x )
				{
					std::cout << "Exception on third get: " << x.what()
							<< std::endl;
					if( so_5::rc_max_overlimit_reaction_deep != x.error_code() )
					{
						std::cout << "This is UNEXPECTED exception" << std::endl;
						throw;
					}
					else
						std::cout << "This is EXPECTED exception" << std::endl;
				}

				if( m_expected_response == responses )
					so_deregister_agent_coop_normally();
			} );
	}

	virtual void
	so_evt_start() override
	{
		auto svc = m_target_mbox->get_one< std::string >();

		m_r1 = svc.async< msg_request >();
		m_r2 = svc.async< msg_request >();
		m_r3 = svc.async< msg_request >();

		so_5::send_to_agent< msg_get_results >( *this );
	}

private :
	struct msg_get_results : public so_5::rt::signal_t {};

	const so_5::rt::mbox_t m_target_mbox;
	const std::string m_expected_response;

	std::future< std::string > m_r1;
	std::future< std::string > m_r2;
	std::future< std::string > m_r3;
};

void
init( so_5::rt::environment_t & env )
{
	auto coop = env.create_coop( so_5::autoname );
	
	auto w = coop->make_agent< a_worker_t >( "[one]" );

	coop->make_agent< a_manager_t >( w->so_direct_mbox(), "[one][one]" );

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
			"simple too deep service request redirect on MPSC-mboxes test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

