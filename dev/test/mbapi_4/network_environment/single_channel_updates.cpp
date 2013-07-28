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

void
single_channel_updates_test_impl(
	UT_CONTEXT_DECL )
{
	network_environment_t ne;

	available_endpoint_table_t aep;
	available_stagepoint_table_t asp;
	channel_info_table_t ci;
	endpoint_list_t el;

	{
		// Добавляем канал, в котором ничего нет.

		// Канал должен быть создан.
		// Но информации никакой пока не будет.
		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			available_endpoint_table_t(),
			available_stagepoint_table_t() );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 0, aep.size() );
		UT_CHECK_EQ( 0, asp.size() );
		UT_CHECK_EQ( 1, ci.size() );
		channel_info_table_t::iterator it = ci.find( channel_uid_wrapper_t( "1" ) );
		UT_CHECK_CONDITION( ci.end() != it );
		channel_info_t & c = *(it->second);

		UT_CHECK_EQ( "uid-1", c.node_uid().str() );
		UT_CHECK_EQ( 0, c.endpoints().size() );
		UT_CHECK_EQ( 0, c.stagepoints().size() );

		ne.delete_channel( channel_uid_wrapper_t( "1" ) );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 0, aep.size() );
		UT_CHECK_EQ( 0, asp.size() );
		UT_CHECK_EQ( 0, ci.size() );
	}

	// -------------------------------------------------------------------
	// -------------------------------------------------------------------
	// -------------------------------------------------------------------
	// В первой итерации добавляем канал, в котором один endpoint,
	// а затем его stagepoint-ы. Затем обновим все то же самое(несколько раз),
	// все должно оставться, как и раньше.
	// -------------------------------------------------------------------
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 1, aep.size() );
		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES(
					"ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 4, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// -------------------------------------------------------------------
	// Уберем stagepoint ep1-stage2
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			// STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 1, aep.size() );

		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES(
					"ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 3, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			// STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					// EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// -------------------------------------------------------------------
	// Добавляем endpoint.
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_DESC_END()

			ENDPOINT_DESC_BEGIN( "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
				ENDPOINT_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			// STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )

			STAGEPOINT_DESC( "endpoint2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			STAGEPOINT_DESC( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			STAGEPOINT_DESC( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();


		UT_CHECK_EQ( 2, aep.size() );

		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES(
					"ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_CHECK_END()

			ENDPOINT_CHECK_BEGIN( "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
				ENDPOINT_CHECK_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 6, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			// STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

			STAGEPOINT_CHECK( "endpoint2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			STAGEPOINT_CHECK( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			STAGEPOINT_CHECK( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
					EXP_ENDPOINT( "endpoint2" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					// EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )

					EXP_STAGE( "endpoint2", "endpoint2" )
					EXP_STAGE( "ep2-stage1", "endpoint2" )
					EXP_STAGE( "ep2-stage2", "endpoint2" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// -------------------------------------------------------------------
	// Добавляем пропущенный stage.
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_DESC_END()

			ENDPOINT_DESC_BEGIN( "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
				ENDPOINT_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )

			STAGEPOINT_DESC( "endpoint2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			STAGEPOINT_DESC( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			STAGEPOINT_DESC( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 2, aep.size() );

		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES(
					"ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_CHECK_END()

			ENDPOINT_CHECK_BEGIN( "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
				ENDPOINT_CHECK_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 7, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

			STAGEPOINT_CHECK( "endpoint2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			STAGEPOINT_CHECK( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			STAGEPOINT_CHECK( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
					EXP_ENDPOINT( "endpoint2" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )

					EXP_STAGE( "endpoint2", "endpoint2" )
					EXP_STAGE( "ep2-stage1", "endpoint2" )
					EXP_STAGE( "ep2-stage2", "endpoint2" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// -------------------------------------------------------------------
	// Изменяем stagepoint-ы endpoint2.
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_DESC_END()

			ENDPOINT_DESC_BEGIN( "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
				ENDPOINT_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )

			STAGEPOINT_DESC( "endpoint2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			STAGEPOINT_DESC( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "3" ), "uid-3", 2 )
			STAGEPOINT_DESC( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "4" ), "uid-4", 3 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 2, aep.size() );

		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES(
					"ep1-stage1" << "ep1-stage2" << "ep1-stage3" )
			ENDPOINT_CHECK_END()

			ENDPOINT_CHECK_BEGIN( "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
				ENDPOINT_CHECK_STAGES( "ep2-stage1" << "ep2-stage2" )
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 7, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )

			STAGEPOINT_CHECK( "endpoint2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
			STAGEPOINT_CHECK( "ep2-stage1", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-3", 3 )
			STAGEPOINT_CHECK( "ep2-stage2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-4", 4 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
					EXP_ENDPOINT( "endpoint2" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )

					EXP_STAGE( "endpoint2", "endpoint2" )
					EXP_STAGE( "ep2-stage1", "endpoint2" )
					EXP_STAGE( "ep2-stage2", "endpoint2" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// -------------------------------------------------------------------
	// Изменяем endpoint1 и endpoint2.
	for( int i = 0; i < 10; ++i )
	{

		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 0 )
				ENDPOINT_STAGES( "ep1-stage1"
					<< "ep1-stage2" << "ep1-stage3" << "ep1-stage4"
					<< "ep1-stage5" << "ep1-stage6" << "ep1-stage7"
					<< "ep1-stage8" << "ep1-stage9" << "ep1-stage10" )
			ENDPOINT_DESC_END()

			ENDPOINT_DESC_BEGIN( "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage4", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage5", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage6", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage7", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage8", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage9", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage10", "endpoint1", channel_uid_wrapper_t( "10" ), "uid-10", 9 )

			STAGEPOINT_DESC( "endpoint2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			// STAGEPOINT_DESC( "ep2-stage1", "endpoint2", 3, "uid-2", 2 )
			// STAGEPOINT_DESC( "ep2-stage2", "endpoint2", 4, "uid-2", 3 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		ne.utest_get_endpoint_table();
		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 2, aep.size() );
		ENDPOINTS_CHECK_BEGIN( aep, el )

			ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				ENDPOINT_CHECK_STAGES( "ep1-stage1"
					<< "ep1-stage2" << "ep1-stage3" << "ep1-stage4"
					<< "ep1-stage5" << "ep1-stage6" << "ep1-stage7"
					<< "ep1-stage8" << "ep1-stage9" << "ep1-stage10" )
			ENDPOINT_CHECK_END()

			ENDPOINT_CHECK_BEGIN( "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
				ENDPOINT_CHECK_NO_STAGES
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 12, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage4", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage5", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage6", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage7", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage8", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage9", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage10", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-10", 10 )

			STAGEPOINT_CHECK( "endpoint2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					EXP_ENDPOINT( "endpoint1" )
					EXP_ENDPOINT( "endpoint2" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )
					EXP_STAGE( "ep1-stage4", "endpoint1" )
					EXP_STAGE( "ep1-stage5", "endpoint1" )
					EXP_STAGE( "ep1-stage6", "endpoint1" )
					EXP_STAGE( "ep1-stage7", "endpoint1" )
					EXP_STAGE( "ep1-stage8", "endpoint1" )
					EXP_STAGE( "ep1-stage9", "endpoint1" )
					EXP_STAGE( "ep1-stage10", "endpoint1" )

					EXP_STAGE( "endpoint2", "endpoint2" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()
	}

	// ----------------------------------------------------------
	// Убираем endpoint1, но stage-ы остаются.
	for( int i = 0; i < 10; ++i )
	{
		available_endpoint_table_t endpoints;

		ENDPOINTS_ADD_BEGIN( endpoints )
			// ENDPOINT_DESC_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 0 )
				// ENDPOINT_STAGES( "ep1-stage1"
					// << "ep1-stage2" << "ep1-stage3" << "ep1-stage4"
					// << "ep1-stage5" << "ep1-stage6" << "ep1-stage7"
					// << "ep1-stage8" << "ep1-stage9" << "ep1-stage10" )
			// ENDPOINT_DESC_END()

			ENDPOINT_DESC_BEGIN( "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
				ENDPOINT_NO_STAGES
			ENDPOINT_DESC_END()
		ENDPOINTS_ADD_END()

		available_stagepoint_table_t stagepoints;

		STAGEPOINTS_ADD_BEGIN( stagepoints )
			// STAGEPOINT_DESC( "endpoint1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage4", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage5", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage6", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage7", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage8", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage9", "endpoint1", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			STAGEPOINT_DESC( "ep1-stage10", "endpoint1", channel_uid_wrapper_t( "10" ), "uid-10", 9 )

			STAGEPOINT_DESC( "endpoint2", "endpoint2", channel_uid_wrapper_t( "2" ), "uid-2", 1 )
			// STAGEPOINT_DESC( "ep2-stage1", "endpoint2", 3, "uid-2", 2 )
			// STAGEPOINT_DESC( "ep2-stage2", "endpoint2", 4, "uid-2", 3 )
		STAGEPOINTS_ADD_END()

		ne.update_channel(
			channel_uid_wrapper_t( "1" ),
			mbapi_node_uid_t::utest_create_uid( "uid-1" ),
			so_5::rt::mbox_ref_t(),
			endpoints,
			stagepoints );

		aep = ne.utest_get_endpoint_table();
		asp = ne.utest_get_stagepoint_table();
		ci = ne.utest_get_channel_info_table();
		el = ne.query_endpoint_list();

		UT_CHECK_EQ( 1, aep.size() );
		ENDPOINTS_CHECK_BEGIN( aep, el )

			// ENDPOINT_CHECK_BEGIN( "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
				// ENDPOINT_CHECK_STAGES( "ep1-stage1"
					// << "ep1-stage2" << "ep1-stage3" << "ep1-stage4"
					// << "ep1-stage5" << "ep1-stage6" << "ep1-stage7"
					// << "ep1-stage8" << "ep1-stage9" << "ep1-stage10" )
			// ENDPOINT_CHECK_END()

			ENDPOINT_CHECK_BEGIN( "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
				ENDPOINT_CHECK_NO_STAGES
			ENDPOINT_CHECK_END()
		ENDPOINTS_CHECK_END()

		UT_CHECK_EQ( 11, asp.size() );
		STAGEPOINTS_CHECK_BEGIN( asp )
			// STAGEPOINT_CHECK( "endpoint1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage1", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage2", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage3", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage4", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage5", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage6", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage7", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage8", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage9", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			STAGEPOINT_CHECK( "ep1-stage10", "endpoint1", channel_uid_wrapper_t( "1" ), "uid-10", 10 )

			STAGEPOINT_CHECK( "endpoint2", "endpoint2", channel_uid_wrapper_t( "1" ), "uid-2", 2 )
		STAGEPOINTS_CHECK_END()

		UT_CHECK_EQ( 1, ci.size() );
		CHANNELS_CHECK_BEGIN( ci )
			CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

				EXP_ENDPOINT_CHECK_BEGIN()
					// EXP_ENDPOINT( "endpoint1" )
					EXP_ENDPOINT( "endpoint2" )
				EXP_ENDPOINT_CHECK_END()

				EXP_STAGE_CHECK_BEGIN()
					// EXP_STAGE( "endpoint1", "endpoint1" )
					EXP_STAGE( "ep1-stage1", "endpoint1" )
					EXP_STAGE( "ep1-stage2", "endpoint1" )
					EXP_STAGE( "ep1-stage3", "endpoint1" )
					EXP_STAGE( "ep1-stage4", "endpoint1" )
					EXP_STAGE( "ep1-stage5", "endpoint1" )
					EXP_STAGE( "ep1-stage6", "endpoint1" )
					EXP_STAGE( "ep1-stage7", "endpoint1" )
					EXP_STAGE( "ep1-stage8", "endpoint1" )
					EXP_STAGE( "ep1-stage9", "endpoint1" )
					EXP_STAGE( "ep1-stage10", "endpoint1" )

					EXP_STAGE( "endpoint2", "endpoint2" )
				EXP_STAGE_CHECK_END()

			CHANNEL_CHECK_END()
		CHANNELS_CHECK_END()

	}


	ne.delete_channel( channel_uid_wrapper_t( "1" ) );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 0, aep.size() );
	UT_CHECK_EQ( 0, asp.size() );
	UT_CHECK_EQ( 0, ci.size() );

}
