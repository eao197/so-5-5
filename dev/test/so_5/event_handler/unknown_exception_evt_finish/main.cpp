/*
 * Test for unknown exception from so_evt_start.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

class test_agent_t final : public so_5::agent_t
{
public :
	test_agent_t( context_t ctx ) : agent_t{ std::move(ctx) } {}

	void
	so_evt_start() override
	{
		so_deregister_agent_coop_normally();
	}

	void
	so_evt_finish() override
	{
		throw( "boom!" );
	}

	so_5::exception_reaction_t
	so_exception_reaction() const override
	{
		return so_5::shutdown_sobjectizer_on_exception;
	}
};

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( [&]( so_5::environment_t & env ) {
						env.register_agent_as_coop( so_5::autoname,
								env.make_agent< test_agent_t >() );
					} );
			},
			20 );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 2;
	}

	return 0;
}

