/*
 * Test for size-limited mchains without abort_app overflow reaction.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

#include <utest_helper_1/h/helper.hpp>

using namespace std;
using namespace chrono;

namespace props = so_5::mchain_props;

void
do_check_no_wait_drop_newest_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "no_wait, drop_newest, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_without_waiting(
					3,
					memory,
					props::overflow_reaction::drop_newest ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	so_5::send< int >( ch, 4 );

	receive( from(ch).handle_n(4).empty_timeout(so_5::no_wait),
			[]( int i ) { UT_CHECK_CONDITION( i < 4 ); } );

	cout << "OK" << std::endl;
}

void
do_check_no_wait_drop_newest( so_5::rt::environment_t & env )
{
	do_check_no_wait_drop_newest_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_no_wait_drop_newest_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

void
do_check_no_wait_remove_oldest_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "no_wait, remove_oldest, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_without_waiting(
					3,
					memory,
					props::overflow_reaction::remove_oldest ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	so_5::send< int >( ch, 4 );

	receive( from(ch).handle_n(4).empty_timeout(so_5::no_wait),
			[]( int i ) { UT_CHECK_CONDITION( i > 1 ); } );

	cout << "OK" << std::endl;
}

void
do_check_no_wait_remove_oldest( so_5::rt::environment_t & env )
{
	do_check_no_wait_remove_oldest_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_no_wait_remove_oldest_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

void
do_check_no_wait_throw_exception_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "no_wait, throw_exception, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_without_waiting(
					3,
					memory,
					props::overflow_reaction::throw_exception ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	try
	{
		so_5::send< int >( ch, 4 );
		UT_CHECK_CONDITION( !"An exception must be throw before this line!" );
	}
	catch( const so_5::exception_t & ex )
	{
		UT_CHECK_CONDITION( so_5::rc_msg_chain_overflow == ex.error_code() );
	}

	cout << "OK" << std::endl;
}

void
do_check_no_wait_throw_exception( so_5::rt::environment_t & env )
{
	do_check_no_wait_throw_exception_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_no_wait_throw_exception_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

UT_UNIT_TEST( test_no_wait )
{
	run_with_time_limit(
		[] {
			so_5::wrapped_env_t env;

			do_check_no_wait_drop_newest( env.environment() );
			do_check_no_wait_remove_oldest( env.environment() );
			do_check_no_wait_throw_exception( env.environment() );
		},
		4,
		"test_no_wait" );
}

const chrono::milliseconds wait_timeout{ 100 };
const chrono::milliseconds timeout_delta{ 20 };

template< typename L >
void check_pause( L lambda )
{
	auto s = chrono::steady_clock::now();
	lambda();
	auto f = chrono::steady_clock::now();

	UT_CHECK_CONDITION( s + (wait_timeout - timeout_delta) < f );
}

void
do_check_wait_drop_newest_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "wait, drop_newest, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_with_waiting(
					3,
					memory,
					props::overflow_reaction::drop_newest,
					wait_timeout ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	check_pause( [&] { so_5::send< int >( ch, 4 ); } );

	receive( from(ch).handle_n(4).empty_timeout(so_5::no_wait),
			[]( int i ) { UT_CHECK_CONDITION( i < 4 ); } );

	cout << "OK" << std::endl;
}

void
do_check_wait_drop_newest( so_5::rt::environment_t & env )
{
	do_check_wait_drop_newest_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_wait_drop_newest_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

void
do_check_wait_remove_oldest_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "wait, remove_oldest, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_with_waiting(
					3,
					memory,
					props::overflow_reaction::remove_oldest,
					wait_timeout ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	check_pause( [&] { so_5::send< int >( ch, 4 ); } );

	receive( from(ch).handle_n(4).empty_timeout(so_5::no_wait),
			[]( int i ) { UT_CHECK_CONDITION( i > 1 ); } );

	cout << "OK" << std::endl;
}

void
do_check_wait_remove_oldest( so_5::rt::environment_t & env )
{
	do_check_wait_remove_oldest_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_wait_remove_oldest_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

void
do_check_wait_throw_exception_impl(
	so_5::rt::environment_t & env,
	const char * case_name,
	props::storage_memory memory )
{
	cout << "wait, throw_exception, " << case_name << ": " << std::flush;

	auto ch = env.create_mchain( so_5::mchain_params{
			props::capacity::make_limited_with_waiting(
					3,
					memory,
					props::overflow_reaction::throw_exception,
					wait_timeout ) } );

	so_5::send< int >( ch, 1 );
	so_5::send< int >( ch, 2 );
	so_5::send< int >( ch, 3 );
	check_pause( [&] {
		try
		{
			so_5::send< int >( ch, 4 );
			UT_CHECK_CONDITION( !"An exception must be throw before this line!" );
		}
		catch( const so_5::exception_t & ex )
		{
			UT_CHECK_CONDITION( so_5::rc_msg_chain_overflow == ex.error_code() );
		}
	} );

	cout << "OK" << std::endl;
}

void
do_check_wait_throw_exception( so_5::rt::environment_t & env )
{
	do_check_wait_throw_exception_impl(
			env, "dynamic", props::storage_memory::dynamic );
	do_check_wait_throw_exception_impl(
			env, "prealloc", props::storage_memory::preallocated );
}

UT_UNIT_TEST( test_wait )
{
	run_with_time_limit(
		[] {
			so_5::wrapped_env_t env;

			do_check_wait_drop_newest( env.environment() );
			do_check_wait_remove_oldest( env.environment() );
			do_check_wait_throw_exception( env.environment() );
		},
		4,
		"test_wait" );
}

int
main()
{
	UT_RUN_UNIT_TEST( test_no_wait )
	UT_RUN_UNIT_TEST( test_wait )

	return 0;
}

