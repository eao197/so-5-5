/*
 * Test for various variants of environment_t::build_coop.
 */

#include <iostream>
#include <map>
#include <exception>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>
		
so_5::rt::mbox_t
manager_mbox( so_5::rt::environment_t & env )
{
	return env.create_local_mbox( "manager" );
}

struct msg_started : public so_5::rt::signal_t {};

class a_manager_t : public so_5::rt::agent_t
{
public :
	a_manager_t(
		context_t ctx,
		unsigned int expected )
		:	so_5::rt::agent_t( ctx )
		,	m_expected( expected )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event< msg_started >(
			manager_mbox( so_environment() ),
			[this] {
				++m_received;
				if( m_received == m_expected )
					so_environment().stop();
			} );
	}

private :
	const unsigned int m_expected;
	unsigned int m_received = { 0 };
};

void
define_agent(
	so_5::rt::environment_t & env,
	so_5::rt::agent_coop_t & coop )
{
	coop.define_agent().on_start( [&env] {
		so_5::send< msg_started >( manager_mbox( env ) );
	} );
}

void
init( so_5::rt::environment_t & env )
{
	using namespace so_5::rt;

	env.register_agent_as_coop( "main", env.make_agent< a_manager_t >( 1 ) );

	env.build_coop( [&env]( agent_coop_t & coop ) {
		define_agent( env, coop );
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
			"build_coop test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

