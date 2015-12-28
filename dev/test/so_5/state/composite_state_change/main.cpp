/*
 * A very simple test case for changing state to a composite state.
 */

#include <iostream>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

class a_test_t final : public so_5::agent_t
{
	state_t st_top_1 = { this, "top_1" };
	state_t st_child_1_1 = { so_5::initial_substate_of{ st_top_1 }, "child_1" };
	state_t st_child_1_2 = { so_5::substate_of{ st_top_1 }, "child_2" };

	state_t st_top_2 = { this, "top_2" };
	state_t st_child_2_1 = { so_5::substate_of{ st_top_2 }, "child_1" };
	state_t st_child_2_2 = { so_5::substate_of{ st_top_2 }, "child_2" };

public :
	a_test_t( context_t ctx )
		:	so_5::agent_t{ ctx }
	{
		this >>= st_top_1;
	}

	virtual void
	so_evt_start() override
	{
		if( !(st_child_1_1 == so_current_state()) )
			throw std::runtime_error( "unexpected current state, expected: " +
					st_child_1_1.query_name() + ", actual: " +
					so_current_state().query_name() );

		try
		{
			this >>= st_top_2;
			throw std::runtime_error( "exception must be throw on attempt "
					"to change state to st_top_2!" );
		}
		catch( const so_5::exception_t & ex )
		{
			std::cout << "Exception: " << ex.what() << std::endl;
			if( so_5::rc_no_initial_substate != ex.error_code() )
				throw std::runtime_error( "expected error_code: " +
						std::to_string( ex.error_code() ) );
		}

		so_deregister_agent_coop_normally();
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
				so_5::launch( []( so_5::environment_t & env ) {
						env.introduce_coop( []( so_5::coop_t & coop ) {
								coop.make_agent< a_test_t >();
							} );
					}/*,
					[]( so_5::environment_params_t & params ) {
						params.message_delivery_tracer(
								so_5::msg_tracing::std_cout_tracer() );
					}*/ );
			},
			4,
			"simple test for switching to a composite state" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

