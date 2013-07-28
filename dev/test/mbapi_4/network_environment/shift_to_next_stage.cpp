/*
	Тестирование mbapi_4::impl::network_environment_t.
*/

#include <iostream>
#include <sstream>

#include <cpp_util_2/h/defs.hpp>

#include <utest_helper_1/h/helper.hpp>

#include <so_5/h/exception.hpp>

#include <mbapi_4/impl/infrastructure/h/network_environment.hpp>

#include <test/mbapi_4/network_environment/h/tests.hpp>

using namespace mbapi_4;
using namespace mbapi_4::impl::infrastructure;

namespace /* ananymous */
{
struct channel_tables_t
{
	available_endpoint_table_t m_endpoints;
	available_stagepoint_table_t m_stagepoints;
};

} /* ananymous namespace */

void
shift_to_next_stage_test_impl(
	UT_CONTEXT_DECL )
{
	{
		network_environment_t ne;
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "A", channel_uid_wrapper_t( "" ), "uid-1", 1 )
				ENDPOINT_STAGES( "a1" << "a2" << "a3" )
			ENDPOINT_DESC_END()
			ENDPOINT_DESC_BEGIN( "B", channel_uid_wrapper_t( "" ), "uid-1", 2 )
				ENDPOINT_STAGES( "b1" << "b2" << "b3" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		{
			const endpoint_t from( "C" );
			const endpoint_t to( "D" );
			stagepoint_t sp( "a2" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "b2" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "unknown_stage" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "unknown_stage" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "A" , "A");

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a3", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a2", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a1", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b1", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b2", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b3", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "B", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}
	}

	{
		network_environment_t ne;
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "A", channel_uid_wrapper_t( "" ), "uid-1", 1 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
			ENDPOINT_DESC_BEGIN( "B", channel_uid_wrapper_t( "1" ), "uid-1", 2 )
				ENDPOINT_STAGES( "b1" << "b2" << "b3" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		{
			const endpoint_t from( "C" );
			const endpoint_t to( "D" );
			stagepoint_t sp( "A" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "b2" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "unknown_stage" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "unknown_stage" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "A" , "A");

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b1", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b2", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "b3", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "B", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

	}

	{
		network_environment_t ne;
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "A", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_STAGES( "a1" << "a2" << "a3" )
			ENDPOINT_DESC_END()
			ENDPOINT_DESC_BEGIN( "B", channel_uid_wrapper_t( "1" ), "uid-1", 2 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		{
			const endpoint_t from( "C" );
			const endpoint_t to( "D" );
			stagepoint_t sp( "a2" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "B" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "unknown_stage" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "unknown_stage" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "A" , "A");

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a3", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a2", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "a1", sp.name() );
			UT_CHECK_EQ( "A", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "B", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}
	}

	{
		network_environment_t ne;
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "A", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
			ENDPOINT_DESC_BEGIN( "B", channel_uid_wrapper_t( "1" ), "uid-1", 2 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		{
			const endpoint_t from( "C" );
			const endpoint_t to( "D" );
			stagepoint_t sp( "A" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "B" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "unknown_stage" , "A");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
			sp = stagepoint_t( "unknown_stage" , "B");
			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}

		{
			const endpoint_t from( "A" );
			const endpoint_t to( "B" );
			stagepoint_t sp( "A" , "A");

			UT_CHECK_CONDITION(
				true == ne.shift_to_next_stage( from, to, sp ) );
			UT_CHECK_EQ( "B", sp.name() );
			UT_CHECK_EQ( "B", sp.endpoint_name() );

			UT_CHECK_CONDITION(
				false == ne.shift_to_next_stage( from, to, sp ) );
		}
	}
}
