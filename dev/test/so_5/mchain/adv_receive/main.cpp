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
				so_5::mchain_props::extraction_status::msg_extracted == r.status() );
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

				do_check_total_time(
						env.environment().create_mchain( p.second ) );
			},
			4,
			"test_total_time: " + p.first );
	}
}

void
do_check_handle_n(
	const so_5::mchain & ch1,
	const so_5::mchain & ch2 )
{
	std::thread child{ [&] {
		auto r = receive(
				from( ch1 ).handle_n( 3 ),
				so_5::handler( [&ch2]( int i ) {
					so_5::send< int >( ch2->as_mbox(), i );
				} ) );

		UT_CHECK_CONDITION( 3 == r.extracted() );
		UT_CHECK_CONDITION( 3 == r.handled() );
	} };

	so_5::send< int >( ch1->as_mbox(), 0 );
	auto r = receive(
			from( ch2 ).handle_n( 2 ),
			so_5::handler( [&ch1]( int i ) {
				so_5::send< int >( ch1->as_mbox(), i + 1 );
			} ) );

	UT_CHECK_CONDITION( 2 == r.extracted() );
	UT_CHECK_CONDITION( 2 == r.handled() );

	child.join();
}

UT_UNIT_TEST( test_handle_n )
{
	auto params = build_mchain_params();
	for( const auto & p : params )
	{
		cout << "=== " << p.first << " ===" << endl;

		run_with_time_limit(
			[&p]()
			{
				so_5::wrapped_env_t env;

				do_check_handle_n(
						env.environment().create_mchain( p.second ),
						env.environment().create_mchain( p.second ) );
			},
			4,
			"test_handle_n: " + p.first );
	}
}

void
do_check_extract_n(
	const so_5::mchain & ch1,
	const so_5::mchain & ch2 )
{
	std::thread child{ [&] {
		auto r = receive(
				from( ch1 ).handle_n( 3 ).extract_n( 3 ),
				so_5::handler( [&ch2]( int i ) {
					so_5::send< int >( ch2->as_mbox(), i );
				} ) );

		UT_CHECK_CONDITION( 3 == r.extracted() );
		UT_CHECK_CONDITION( 1 == r.handled() );
	} };

	so_5::send< string >( ch1->as_mbox(), "0" );
	so_5::send< int >( ch1->as_mbox(), 0 );

	auto r = receive(
			from( ch2 ).handle_n( 1 ),
			so_5::handler( [&ch1]( int i ) {
				so_5::send< string >( ch1->as_mbox(), to_string( i + 1 ) );
				so_5::send< int >( ch1->as_mbox(), i + 1 );
			} ) );

	UT_CHECK_CONDITION( 1 == r.extracted() );
	UT_CHECK_CONDITION( 1 == r.handled() );

	child.join();
}

UT_UNIT_TEST( test_extract_n )
{
	auto params = build_mchain_params();
	for( const auto & p : params )
	{
		cout << "=== " << p.first << " ===" << endl;

		run_with_time_limit(
			[&p]()
			{
				so_5::wrapped_env_t env;

				do_check_extract_n(
						env.environment().create_mchain( p.second ),
						env.environment().create_mchain( p.second ) );
			},
			4,
			"test_extract_n: " + p.first );
	}
}

int
main()
{
	UT_RUN_UNIT_TEST( test_timeout_on_empty_queue )
	UT_RUN_UNIT_TEST( test_total_time )
	UT_RUN_UNIT_TEST( test_handle_n )
	UT_RUN_UNIT_TEST( test_extract_n )

	return 0;
}

