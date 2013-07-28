/*
	Тестирование mbapi_4::impl::network_environment_t.
*/

#include <iostream>
#include <sstream>

#include <cpp_util_2/h/defs.hpp>

#include <utest_helper_1/h/helper.hpp>

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
multiple_channel_updates_test_impl(
	UT_CONTEXT_DECL )
{
	network_environment_t ne;
	available_endpoint_table_t aep;
	available_stagepoint_table_t asp;
	channel_info_table_t ci;
	endpoint_list_t el;

	// Наш узел с0 появляется и к нам подсоединяется узел с1.

	channel_tables_t c1;
	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.1/1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.1/2", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
		STAGEPOINT_DESC(  "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "" ), "uid-1", 0 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 2, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 2, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется второй с1 устанавливает связь с узлом c2.
	// Поэтому в его таблице появляются endpoint-ы и stage-ы
	// из узла c2.

	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.2/1", channel_uid_wrapper_t( "10" ), "uid-2", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.2/2", channel_uid_wrapper_t( "10" ), "uid-2", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "10" ), "uid-2", 1 )
		STAGEPOINT_DESC(  "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "10" ), "uid-2", 1 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 4, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 4, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется с3 и устанавливает связь с узлом c2.
	// Поэтому в его таблице появляются endpoint-ы и stage-ы
	// из узла c3, которые затем появляются в узле с1.

	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.3/1", channel_uid_wrapper_t( "10" ), "uid-3", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.3/2", channel_uid_wrapper_t( "10" ), "uid-3", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "10" ), "uid-3", 2 )
		STAGEPOINT_DESC(  "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "10" ), "uid-3", 2 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 6, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 6, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется с4, который связан с с2 и с3.
	// Его point-ы будут видны чересз с1-с2 (с1-с2-с3 - длиннее).

	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.4/1", channel_uid_wrapper_t( "10" ), "uid-4", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.4/2", channel_uid_wrapper_t( "10" ), "uid-4", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "10" ), "uid-4", 2 )
		STAGEPOINT_DESC(  "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "10" ), "uid-4", 2 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 8, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 8, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )

		STAGEPOINT_CHECK( "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
		STAGEPOINT_CHECK( "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )

				EXP_ENDPOINT( "ep.4/1" )
				EXP_ENDPOINT( "ep.4/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )

				EXP_STAGE( "ep.4/1", "ep.4/1" )
				EXP_STAGE( "ep.4/2", "ep.4/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется с5, который связан с с2.

	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.5/1", channel_uid_wrapper_t( "10" ), "uid-5", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.5/2", channel_uid_wrapper_t( "10" ), "uid-5", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "10" ), "uid-5", 2 )
		STAGEPOINT_DESC(  "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "10" ), "uid-5", 2 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 10, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 10, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )

		STAGEPOINT_CHECK( "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
		STAGEPOINT_CHECK( "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )

		STAGEPOINT_CHECK( "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 3 )
		STAGEPOINT_CHECK( "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 3 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )

				EXP_ENDPOINT( "ep.4/1" )
				EXP_ENDPOINT( "ep.4/2" )

				EXP_ENDPOINT( "ep.5/1" )
				EXP_ENDPOINT( "ep.5/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )

				EXP_STAGE( "ep.4/1", "ep.4/1" )
				EXP_STAGE( "ep.4/2", "ep.4/2" )

				EXP_STAGE( "ep.5/1", "ep.5/1" )
				EXP_STAGE( "ep.5/2", "ep.5/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется связь между с5 и с1.

	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.5/1", channel_uid_wrapper_t( "11" ), "uid-5", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.5/2", channel_uid_wrapper_t( "11" ), "uid-5", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "11" ), "uid-5", 1 )
		STAGEPOINT_DESC(  "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "11" ), "uid-5", 1 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "1" ),
		mbapi_node_uid_t::utest_create_uid( "uid-1" ),
		so_5::rt::mbox_ref_t(),
		c1.m_endpoints,
		c1.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 10, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 10, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )

		STAGEPOINT_CHECK( "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "1" ), "uid-4", 3 )
		STAGEPOINT_CHECK( "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "1" ), "uid-4", 3 )

		STAGEPOINT_CHECK( "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
		STAGEPOINT_CHECK( "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )

				EXP_ENDPOINT( "ep.4/1" )
				EXP_ENDPOINT( "ep.4/2" )

				EXP_ENDPOINT( "ep.5/1" )
				EXP_ENDPOINT( "ep.5/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )

				EXP_STAGE( "ep.4/1", "ep.4/1" )
				EXP_STAGE( "ep.4/2", "ep.4/2" )

				EXP_STAGE( "ep.5/1", "ep.5/1" )
				EXP_STAGE( "ep.5/2", "ep.5/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется связь между с0 и с3.
	channel_tables_t c3;

	ENDPOINTS_ADD_BEGIN( c3.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.3/1", channel_uid_wrapper_t( "" ), "uid-3", 0 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.3/2", channel_uid_wrapper_t( "" ), "uid-3", 0 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.4/1", channel_uid_wrapper_t( "40" ), "uid-4", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.4/2", channel_uid_wrapper_t( "40" ), "uid-4", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.2/1", channel_uid_wrapper_t( "20" ), "uid-2", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.2/2", channel_uid_wrapper_t( "20" ), "uid-2", 1 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.1/1", channel_uid_wrapper_t( "20" ), "uid-1", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.1/2", channel_uid_wrapper_t( "20" ), "uid-1", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.5/1", channel_uid_wrapper_t( "20" ), "uid-5", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
		ENDPOINT_DESC_BEGIN( "ep.5/2", channel_uid_wrapper_t( "20" ), "uid-5", 2 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c3.m_stagepoints )
		STAGEPOINT_DESC(  "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "" ), "uid-3", 0 )
		STAGEPOINT_DESC(  "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "" ), "uid-3", 0 )

		STAGEPOINT_DESC(  "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "40" ), "uid-4", 1 )
		STAGEPOINT_DESC(  "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "40" ), "uid-4", 1 )

		STAGEPOINT_DESC(  "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "20" ), "uid-2", 1 )
		STAGEPOINT_DESC(  "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "20" ), "uid-2", 1 )

		STAGEPOINT_DESC(  "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "20" ), "uid-1", 2 )
		STAGEPOINT_DESC(  "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "20" ), "uid-1", 2 )

		STAGEPOINT_DESC(  "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "20" ), "uid-5", 2 )
		STAGEPOINT_DESC(  "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "20" ), "uid-5", 2 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "2" ),
		mbapi_node_uid_t::utest_create_uid( "uid-3" ),
		so_5::rt::mbox_ref_t(),
		c3.m_endpoints,
		c3.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 10, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.4/1", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.4/2", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 10, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "2" ), "uid-3", 1 )

		STAGEPOINT_CHECK( "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
		STAGEPOINT_CHECK( "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "2" ), "uid-4", 2 )

		STAGEPOINT_CHECK( "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
		STAGEPOINT_CHECK( "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 2, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.5/1" )
				EXP_ENDPOINT( "ep.5/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.5/1", "ep.5/1" )
				EXP_STAGE( "ep.5/2", "ep.5/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()

		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "2" ), "uid-3" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )

				EXP_ENDPOINT( "ep.4/1" )
				EXP_ENDPOINT( "ep.4/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )

				EXP_STAGE( "ep.4/1", "ep.4/1" )
				EXP_STAGE( "ep.4/2", "ep.4/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// Появляется c6/c7 связанные с с0.
	channel_tables_t c6;
	channel_tables_t c7;

	ENDPOINTS_ADD_BEGIN( c6.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.6", channel_uid_wrapper_t( "" ), "uid-6", 0 )
			ENDPOINT_STAGES( "sp.6-1" << "sp.6-2" << "sp.6-3" )
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.7", channel_uid_wrapper_t( "70" ), "uid-7", 1 )
			ENDPOINT_STAGES( "sp.7-1" << "sp.7-2" << "sp.7-3" )
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	ENDPOINTS_ADD_BEGIN( c7.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep.6", channel_uid_wrapper_t( "60" ), "uid-6", 1 )
			ENDPOINT_STAGES( "sp.6-1" << "sp.6-2" << "sp.6-3" )
		ENDPOINT_DESC_END()

		ENDPOINT_DESC_BEGIN( "ep.7", channel_uid_wrapper_t( "" ), "uid-7", 0 )
			ENDPOINT_STAGES( "sp.7-1" << "sp.7-2" << "sp.7-3" )
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c6.m_stagepoints )
		STAGEPOINT_DESC(  "ep.6", "ep.6", channel_uid_wrapper_t( "" ), "uid-6", 0 )
		STAGEPOINT_DESC(  "sp.6-1", "ep.6", channel_uid_wrapper_t( "" ), "uid-6", 0 )
		STAGEPOINT_DESC(  "sp.6-2", "ep.6", channel_uid_wrapper_t( "" ), "uid-6", 0 )
		STAGEPOINT_DESC(  "sp.6-3", "ep.6", channel_uid_wrapper_t( "" ), "uid-6", 0 )

		STAGEPOINT_DESC(  "ep.7", "ep.7", channel_uid_wrapper_t( "70" ), "uid-7", 1 )
		STAGEPOINT_DESC(  "sp.7-1", "ep.7", channel_uid_wrapper_t( "70" ), "uid-7", 1 )
		STAGEPOINT_DESC(  "sp.7-2", "ep.7", channel_uid_wrapper_t( "70" ), "uid-7", 1 )
		STAGEPOINT_DESC(  "sp.7-3", "ep.7", channel_uid_wrapper_t( "70" ), "uid-7", 1 )
	STAGEPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c7.m_stagepoints )
		STAGEPOINT_DESC(  "ep.6", "ep.6", channel_uid_wrapper_t( "60" ), "uid-6", 1 )
		STAGEPOINT_DESC(  "sp.6-1", "ep.6", channel_uid_wrapper_t( "60" ), "uid-6", 1 )
		STAGEPOINT_DESC(  "sp.6-2", "ep.6", channel_uid_wrapper_t( "60" ), "uid-6", 1 )
		STAGEPOINT_DESC(  "sp.6-3", "ep.6", channel_uid_wrapper_t( "60" ), "uid-6", 1 )

		STAGEPOINT_DESC(  "ep.7", "ep.7", channel_uid_wrapper_t( "" ), "uid-7", 0 )
		STAGEPOINT_DESC(  "sp.7-1", "ep.7", channel_uid_wrapper_t( "" ), "uid-7", 0 )
		STAGEPOINT_DESC(  "sp.7-2", "ep.7", channel_uid_wrapper_t( "" ), "uid-7", 0 )
		STAGEPOINT_DESC(  "sp.7-3", "ep.7", channel_uid_wrapper_t( "" ), "uid-7", 0 )
	STAGEPOINTS_ADD_END()

	ne.update_channel(
		channel_uid_wrapper_t( "6" ),
		mbapi_node_uid_t::utest_create_uid( "uid-6" ),
		so_5::rt::mbox_ref_t(),
		c6.m_endpoints,
		c6.m_stagepoints );

	ne.update_channel(
		channel_uid_wrapper_t( "7" ),
		mbapi_node_uid_t::utest_create_uid( "uid-7" ),
		so_5::rt::mbox_ref_t(),
		c7.m_endpoints,
		c7.m_stagepoints );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 12, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )

		ENDPOINT_CHECK_BEGIN( "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.3/1", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.3/2", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.4/1", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.4/2", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()

		ENDPOINT_CHECK_BEGIN( "ep.6", channel_uid_wrapper_t( "6" ), "uid-6", 1 )
			ENDPOINT_CHECK_STAGES( "sp.6-1" << "sp.6-2" << "sp.6-3" )
		ENDPOINT_CHECK_END()
		ENDPOINT_CHECK_BEGIN( "ep.7", channel_uid_wrapper_t( "7" ), "uid-7", 1 )
			ENDPOINT_CHECK_STAGES( "sp.7-1" << "sp.7-2" << "sp.7-3" )
		ENDPOINT_CHECK_END()

	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 18, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep.1/1", "ep.1/1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINT_CHECK( "ep.1/2", "ep.1/2", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

		STAGEPOINT_CHECK( "ep.2/1", "ep.2/1", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINT_CHECK( "ep.2/2", "ep.2/2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )

		STAGEPOINT_CHECK( "ep.3/1", "ep.3/1", channel_uid_wrapper_t( "2" ), "uid-3", 1 )
		STAGEPOINT_CHECK( "ep.3/2", "ep.3/2", channel_uid_wrapper_t( "2" ), "uid-3", 1 )

		STAGEPOINT_CHECK( "ep.4/1", "ep.4/1", channel_uid_wrapper_t( "2" ), "uid-4", 2 )
		STAGEPOINT_CHECK( "ep.4/2", "ep.4/2", channel_uid_wrapper_t( "2" ), "uid-4", 2 )

		STAGEPOINT_CHECK( "ep.5/1", "ep.5/1", channel_uid_wrapper_t( "1" ), "uid-5", 2 )
		STAGEPOINT_CHECK( "ep.5/2", "ep.5/2", channel_uid_wrapper_t( "1" ), "uid-5", 2 )

		STAGEPOINT_CHECK( "ep.6", "ep.6", channel_uid_wrapper_t( "6" ), "uid-6", 1 )
		STAGEPOINT_CHECK( "sp.6-1", "ep.6", channel_uid_wrapper_t( "6" ), "uid-6", 1 )
		STAGEPOINT_CHECK( "sp.6-2", "ep.6", channel_uid_wrapper_t( "6" ), "uid-6", 1 )
		STAGEPOINT_CHECK( "sp.6-3", "ep.6", channel_uid_wrapper_t( "6" ), "uid-6", 1 )

		STAGEPOINT_CHECK( "ep.7", "ep.7", channel_uid_wrapper_t( "7" ), "uid-7", 1 )
		STAGEPOINT_CHECK( "sp.7-1", "ep.7", channel_uid_wrapper_t( "7" ), "uid-7", 1 )
		STAGEPOINT_CHECK( "sp.7-2", "ep.7", channel_uid_wrapper_t( "7" ), "uid-7", 1 )
		STAGEPOINT_CHECK( "sp.7-3", "ep.7", channel_uid_wrapper_t( "7" ), "uid-7", 1 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 4, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.1/1" )
				EXP_ENDPOINT( "ep.1/2" )

				EXP_ENDPOINT( "ep.2/1" )
				EXP_ENDPOINT( "ep.2/2" )

				EXP_ENDPOINT( "ep.5/1" )
				EXP_ENDPOINT( "ep.5/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.1/1", "ep.1/1" )
				EXP_STAGE( "ep.1/2", "ep.1/2" )

				EXP_STAGE( "ep.2/1", "ep.2/1" )
				EXP_STAGE( "ep.2/2", "ep.2/2" )

				EXP_STAGE( "ep.5/1", "ep.5/1" )
				EXP_STAGE( "ep.5/2", "ep.5/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()

		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "2" ), "uid-3" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.3/1" )
				EXP_ENDPOINT( "ep.3/2" )

				EXP_ENDPOINT( "ep.4/1" )
				EXP_ENDPOINT( "ep.4/2" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.3/1", "ep.3/1" )
				EXP_STAGE( "ep.3/2", "ep.3/2" )

				EXP_STAGE( "ep.4/1", "ep.4/1" )
				EXP_STAGE( "ep.4/2", "ep.4/2" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()

		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "6" ), "uid-6" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.6" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.6", "ep.6" )
				EXP_STAGE( "sp.6-1", "ep.6" )
				EXP_STAGE( "sp.6-2", "ep.6" )
				EXP_STAGE( "sp.6-3", "ep.6" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "7" ), "uid-7" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep.7" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep.7", "ep.7" )
				EXP_STAGE( "sp.7-1", "ep.7" )
				EXP_STAGE( "sp.7-2", "ep.7" )
				EXP_STAGE( "sp.7-3", "ep.7" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

}