/*
 * A simple test for message limits (redirecting message with
 * too deep overlimit reaction level).
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

struct msg_request : public so_5::rt::message_t
{
	const so_5::rt::mbox_t m_reply_to;

	msg_request( so_5::rt::mbox_t reply_to )
		: m_reply_to( std::move( reply_to ) )
	{}
};

struct msg_response : public so_5::rt::message_t
{
	const std::string m_reply;

	msg_response( std::string reply ) : m_reply( std::move( reply ) )
	{}
};

class a_worker_t : public so_5::rt::agent_t
{
public :
	a_worker_t(
		so_5::rt::environment_t & env,
		so_5::rt::mbox_t self_mbox,
		std::string reply )
		:	so_5::rt::agent_t( env,
				tuning_options().message_limits(
					limit_then_redirect< msg_request >( 2,
						[this] { return m_self_mbox; } ) ) )
		,	m_self_mbox( std::move( self_mbox ) )
		,	m_reply( std::move( reply ) )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event(
			m_self_mbox,
			[&]( const msg_request & evt ) {
				so_5::send< msg_response >( evt.m_reply_to, m_reply );
			} );
	}

private :
	const so_5::rt::mbox_t m_self_mbox;
	const std::string m_reply;
};

class a_manager_t : public so_5::rt::agent_t
{
public :
	a_manager_t(
		so_5::rt::environment_t & env,
		so_5::rt::mbox_t self_mbox,
		so_5::rt::mbox_t target_mbox,
		std::string expected_response )
		:	so_5::rt::agent_t( env )
		,	m_self_mbox( std::move( self_mbox ) )
		,	m_target_mbox( std::move( target_mbox ) )
		,	m_expected_response( expected_response )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event(
			m_self_mbox,
			[&]( const msg_response & evt ) {
				m_responses += evt.m_reply;

				if( m_expected_response == m_responses )
					so_deregister_agent_coop_normally();
			} );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send< msg_request >( m_target_mbox, m_self_mbox );
		so_5::send< msg_request >( m_target_mbox, m_self_mbox );

		try
		{
			so_5::send< msg_request >( m_target_mbox, m_self_mbox );
		}
		catch( const so_5::exception_t & x )
		{
			std::cout << "Exception on third send: " << x.what()
					<< std::endl;
			if( so_5::rc_max_overlimit_reaction_deep != x.error_code() )
			{
				std::cout << "This is UNEXPECTED exception" << std::endl;
				throw;
			}
			else
				std::cout << "This is EXPECTED exception" << std::endl;
		}
	}

private :
	const so_5::rt::mbox_t m_self_mbox;
	const so_5::rt::mbox_t m_target_mbox;
	const std::string m_expected_response;
	std::string m_responses;
};

void
init( so_5::rt::environment_t & env )
{
	auto w_mbox = env.create_local_mbox();
	auto m_mbox = env.create_local_mbox();

	auto coop = env.create_coop( so_5::autoname );
	
	coop->make_agent< a_manager_t >( m_mbox, w_mbox, "[one][one]" );

	coop->make_agent< a_worker_t >( w_mbox, "[one]" );

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
			"simple too deep message redirect on MPMC-mboxes test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

