/*
 * A simple test for msg_bag.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include "../bag_params.hpp"

using namespace std;

void
check_drop_content( const so_5::rt::msg_bag_t & bag )
{
	so_5::send< int >( bag->as_mbox(), 0 );
	so_5::send< int >( bag->as_mbox(), 1 );

	close_drop_content( bag );

	auto r = receive(
			bag,
			so_5::rt::msg_bag::clock::duration::zero(),
			so_5::handler( []( int i ) {
				throw runtime_error( "unexpected message: " + to_string(i) );
			} ) );

	if( r != 0 )
		throw runtime_error( "unexpected value of so_5::receive "
				"return code: " + to_string( r ) );
}

void
check_retain_content( const so_5::rt::msg_bag_t & bag )
{
	so_5::send< int >( bag->as_mbox(), 0 );
	so_5::send< int >( bag->as_mbox(), 1 );

	close_retain_content( bag );

	std::size_t r = 0;
	r = receive(
			bag,
			so_5::rt::msg_bag::clock::duration::zero(),
			so_5::handler( []( int i ) {
				if( i != 0 )
					throw runtime_error( "unexpected message: " + to_string(i) );
			} ) );

	if( r != 1 )
		throw runtime_error( "1: unexpected value of so_5::receive "
				"return code: " + to_string( r ) );

	r = receive(
			bag,
			so_5::rt::msg_bag::clock::duration::zero(),
			so_5::handler( []( int i ) {
				if( i != 1 )
					throw runtime_error( "unexpected message: " + to_string(i) );
			} ) );

	if( r != 1 )
		throw runtime_error( "2: unexpected value of so_5::receive "
				"return code: " + to_string( r ) );
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

					check_drop_content(
							env.environment().create_msg_bag( p.second ) );
					check_retain_content(
							env.environment().create_msg_bag( p.second ) );
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

