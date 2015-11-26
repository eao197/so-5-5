/*
 * A simple test for msg_bag.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include "../bag_params.hpp"

using namespace std;

void
do_check( const so_5::rt::msg_bag_t & bag )
{
	auto f1 = so_5::request_future< std::string, int >( bag->as_mbox(), 42 );
	auto f2 = so_5::request_future< std::string, int >( bag->as_mbox(), -1 );

	for( int i = 0; i != 2; ++i )
	{
		auto r = so_5::receive(
				bag,
				so_5::rt::msg_bag::clock::duration::zero(),
				so_5::handler( []( int i ) -> std::string {
					if( i < 0 )
						throw invalid_argument( "negative value" );
					return std::to_string( i );
				} ) );

		if( r != 1 )
			throw runtime_error( "unexpected value of so_5::receive "
					"return code: " + to_string( r ) );
	}

	if( "42" != f1.get() )
		throw runtime_error( "invalid value of f1.get(): " + f1.get() );

	try
	{
		const auto v2 = f2.get();
		throw runtime_error( "value from f2 received: " + v2 );
	}
	catch( const invalid_argument & ex )
	{
		const std::string expected = "negative value";
		if( expected != ex.what() )
			throw;
	}
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::wrapped_env_t env;

				auto params = build_bag_params();

				for( const auto & p : params )
				{
					cout << "=== " << p.first << " ===" << endl;

					do_check( env.environment().create_msg_bag( p.second ) );
				}
			},
			4,
			"simple test for msg_bag" );
	}
	catch( const exception & ex )
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

