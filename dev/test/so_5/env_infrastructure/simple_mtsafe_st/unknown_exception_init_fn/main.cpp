/*
 * A test for simple_mtsafe_st_env_infastructure with unknown
 * exception from init_fn function.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include <utest_helper_1/h/helper.hpp>

using namespace std;

int
main()
{
	try
	{
		run_with_time_limit(
			[]() {
				try
				{
					so_5::launch(
						[]( so_5::environment_t & ) {
							throw "boom!";
						},
						[]( so_5::environment_params_t & params ) {
							params.infrastructure_factory(
									so_5::env_infrastructures::simple_mtsafe::factory() );
						} );

					// An exception should be thrown from so_5::launch.
					std::cout << "We expect an exception from launch";
					std::abort();
				}
				catch( const so_5::exception_t & x )
				{
					std::cout << "Exception is caught: " << x.what() << std::endl;
				}
			},
			5 );
	}
	catch( const exception & ex )
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

