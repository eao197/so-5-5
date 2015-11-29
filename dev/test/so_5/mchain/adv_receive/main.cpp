/*
 * A simple test for mchain.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include <utest_helper_1/h/helper.hpp>

#include "../mchain_params.hpp"

using namespace std;
using namespace chrono;

void
do_check_timeout_on_empty_queue( const so_5::mchain & chain )
{
	std::thread child{ [&] {
		auto r = receive(
				from( chain ).empty_timeout( milliseconds( 500 ) ) );

		UT_CHECK_CONDITION( 0 == r.extracted() );
		UT_CHECK_CONDITION(
				so_5::mchain_props::extraction_status::no_messages == r.status() );
	} };

	child.join();
}

UT_UNIT_TEST( test_timeout_on_empty_queue )
{
	auto params = build_mchain_params();
	for( const auto & p : params )
	{
		cout << "=== " << p.first << " ===" << endl;

		run_with_time_limit(
			[&p]()
			{
				so_5::wrapped_env_t env;

				do_check_timeout_on_empty_queue(
						env.environment().create_mchain( p.second ) );
			},
			4,
			"test_timeout_on_empty_queue: " + p.first );
	}
}

void
do_check_total_time( const so_5::mchain & chain )
{
	so_5::send< int >( chain->as_mbox(), 0 );
	so_5::send< int >( chain->as_mbox(), 1 );
	so_5::send< string >( chain->as_mbox(), "hello!" );

	std::thread child{ [&] {
		auto r = receive(
				from( chain ).total_time( milliseconds( 500 ) ),
				so_5::handler( []( const std::string & ) {} ) );

		UT_CHECK_CONDITION( 3 == r.extracted() );
		UT_CHECK_CONDITION( 1 == r.handled() );
		UT_CHECK_CONDITION(
				so_5::mchain_props::extraction_status::no_messages == r.status() );
	} };

	child.join();
}

UT_UNIT_TEST( test_total_time )
{
	auto params = build_mchain_params();
	for( const auto & p : params )
	{
		cout << "=== " << p.first << " ===" << endl;

		run_with_time_limit(
			[&p]()
			{
				so_5::wrapped_env_t env;

				do_check_timeout_on_empty_queue(
						env.environment().create_mchain( p.second ) );
			},
			4,
			"test_timeout_on_empty_queue: " + p.first );
	}
}

int
main()
{
	UT_RUN_UNIT_TEST( test_timeout_on_empty_queue )
	UT_RUN_UNIT_TEST( test_total_time )

	return 0;
}

