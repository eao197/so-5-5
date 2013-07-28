/*
	Тестирование mbapi_4::impl::network_environment_t.
*/

#include <iostream>
#include <sstream>
#include <iterator>

#include <cpp_util_2/h/defs.hpp>

#include <utest_helper_1/h/helper.hpp>

#include <mbapi_4/impl/infrastructure/h/network_environment.hpp>

#include <test/mbapi_4/network_environment/h/tests.hpp>

using namespace mbapi_4;
using namespace mbapi_4::impl::infrastructure;

UT_UNIT_TEST( create_network_environment )
{
	network_environment_t ne;
}

UT_UNIT_TEST( single_channel_updates )
{
	single_channel_updates_test_impl( UT_CONTEXT );
}

UT_UNIT_TEST( multiple_channel_updates )
{
	multiple_channel_updates_test_impl( UT_CONTEXT );
}

UT_UNIT_TEST( local_node_updates )
{
	local_node_updates_test_impl( UT_CONTEXT );
}

UT_UNIT_TEST( shift_to_next_stage )
{
	shift_to_next_stage_test_impl( UT_CONTEXT );
}

int
main( int argc, char ** argv)
{
	UT_RUN_UNIT_TEST( create_network_environment );
	UT_RUN_UNIT_TEST( single_channel_updates );
	UT_RUN_UNIT_TEST( multiple_channel_updates );
	UT_RUN_UNIT_TEST( local_node_updates );
	UT_RUN_UNIT_TEST( shift_to_next_stage );

	return 0;
}
