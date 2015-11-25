/*
 * A simple test for msg_bag.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include <vector>
#include <utility>
#include <string>

using namespace std;
namespace bag_ns = so_5::rt::msg_bag;

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::wrapped_env_t env;

				vector< pair< string, so_5::rt::bag_params_t > > params;
				params.emplace_back( "unlimited",
						so_5::rt::bag_params_t{
								bag_ns::capacity_t::make_unlimited() } );
				params.emplace_back( "limited(dynamic,nowait)",
						so_5::rt::bag_params_t{
								bag_ns::capacity_t::make_limited_without_waiting(
										5,
										bag_ns::storage_memory_t::dynamic,
										bag_ns::overflow_reaction_t::drop_newest ) } );
				params.emplace_back( "limited(preallocated,nowait)",
						so_5::rt::bag_params_t{
								bag_ns::capacity_t::make_limited_without_waiting(
										5,
										bag_ns::storage_memory_t::preallocated,
										bag_ns::overflow_reaction_t::drop_newest ) } );
				params.emplace_back( "limited(dynamic,wait)",
						so_5::rt::bag_params_t{
								bag_ns::capacity_t::make_limited_with_waiting(
										5,
										bag_ns::storage_memory_t::dynamic,
										bag_ns::overflow_reaction_t::drop_newest,
										chrono::milliseconds(200) ) } );
				params.emplace_back( "limited(preallocated,wait)",
						so_5::rt::bag_params_t{
								bag_ns::capacity_t::make_limited_with_waiting(
										5,
										bag_ns::storage_memory_t::preallocated,
										bag_ns::overflow_reaction_t::drop_newest,
										chrono::milliseconds(200) ) } );

				for( const auto & p : params )
				{
					cout << "=== " << p.first << " ===" << endl;

					auto bag = env.environment().create_msg_bag( p.second );

					so_5::send< int >( bag->as_mbox(), 42 );

					auto r = so_5::receive(
							bag,
							so_5::rt::msg_bag::clock::duration::zero(),
							so_5::handler( []( int i ) {
								if( 42 != i )
									throw runtime_error( "unexpected int-message: "
											+ to_string( i ) );
							} ),
							so_5::handler( []( const std::string & s ) {
								throw runtime_error( "unexpected string msg: " + s );
							} ) );

					if( r != 1 )
						throw runtime_error( "unexpected value of so_5::receive "
								"return code: " + to_string( r ) );
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

