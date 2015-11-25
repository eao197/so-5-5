/*
 * A simple test for msg_bag.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::wrapped_env_t env;

				auto bag = env.environment().create_msg_bag(
						so_5::rt::bag_params_t{
								so_5::rt::msg_bag::capacity_t::make_unlimited() } );

				so_5::send< int >( bag->as_mbox(), 42 );

				auto r = so_5::receive(
						bag,
						so_5::rt::msg_bag::clock::duration::zero(),
						so_5::handler( []( int i ) {
							if( 42 != i )
								throw std::runtime_error( "unexpected int-message: "
										+ std::to_string( i ) );
						} ),
						so_5::handler( []( const std::string & s ) {
							throw std::runtime_error( "unexpected string msg: " + s );
						} ) );

				if( r != 1 )
					throw std::runtime_error( "unexpected value of so_5::receive "
							"return code: " + std::to_string( r ) );
			},
			4,
			"simple test for msg_bag" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

