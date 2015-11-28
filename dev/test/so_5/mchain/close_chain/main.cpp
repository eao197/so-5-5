/*
 * A simple test for mchain.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include "../mchain_params.hpp"

using namespace std;

void
check_drop_content( const so_5::mchain_t & chain )
{
	so_5::send< int >( chain->as_mbox(), 0 );
	so_5::send< int >( chain->as_mbox(), 1 );

	close_drop_content( chain );

	auto r = receive(
			chain,
			so_5::mchain_props::clock::duration::zero(),
			so_5::handler( []( int i ) {
				throw runtime_error( "unexpected message: " + to_string(i) );
			} ) );

	if( r != 0 )
		throw runtime_error( "unexpected value of so_5::receive "
				"return code: " + to_string( r ) );
}

void
check_retain_content( const so_5::mchain_t & chain )
{
	so_5::send< int >( chain->as_mbox(), 0 );
	so_5::send< int >( chain->as_mbox(), 1 );

	close_retain_content( chain );

	std::size_t r = 0;
	r = receive(
			chain,
			so_5::mchain_props::clock::duration::zero(),
			so_5::handler( []( int i ) {
				if( i != 0 )
					throw runtime_error( "unexpected message: " + to_string(i) );
			} ) );

	if( r != 1 )
		throw runtime_error( "1: unexpected value of so_5::receive "
				"return code: " + to_string( r ) );

	r = receive(
			chain,
			so_5::mchain_props::clock::duration::zero(),
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

				auto params = build_mchain_params();

				for( const auto & p : params )
				{
					cout << "=== " << p.first << " ===" << endl;

					check_drop_content(
							env.environment().create_mchain( p.second ) );
					check_retain_content(
							env.environment().create_mchain( p.second ) );
				}
			},
			4,
			"simple test for mchain" );
	}
	catch( const exception & ex )
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

