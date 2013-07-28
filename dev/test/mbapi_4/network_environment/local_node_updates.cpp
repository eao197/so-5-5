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
local_node_updates_test_impl(
	UT_CONTEXT_DECL )
{
	network_environment_t ne;

	available_endpoint_table_t aep;
	available_stagepoint_table_t asp;
	channel_info_table_t ci;
	endpoint_list_t el;

	channel_tables_t c1;
	ENDPOINTS_ADD_BEGIN( c1.m_endpoints )
		ENDPOINT_DESC_BEGIN( "ep", channel_uid_wrapper_t( "" ), "uid-1", 0 )
			ENDPOINT_NO_STAGES
		ENDPOINT_DESC_END()
	ENDPOINTS_ADD_END()

	STAGEPOINTS_ADD_BEGIN( c1.m_stagepoints )
		STAGEPOINT_DESC(  "ep", "ep", channel_uid_wrapper_t( "" ), "uid-1", 0 )
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

	UT_CHECK_EQ( 1, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )
		ENDPOINT_CHECK_BEGIN( "ep", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep", "ep", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep", "ep" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()



	endpoint_t ep( "ep" );
	endpoint_stage_chain_t epsc( ep );
	ne.add_local_endpoint( epsc );

	// При дубликате должно быть выбрашено исключение.
	UT_CHECK_THROW(
		so_5::exception_t,
		ne.add_local_endpoint( epsc );
	);

	stagepoint_t sp( "ep", "ep" );
	ne.add_local_stagepoint( sp );
	UT_CHECK_THROW(
		so_5::exception_t,
		ne.add_local_stagepoint( sp );
	);

	// Обновление с каналом не должно изменить ситуацию.
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

	UT_CHECK_EQ( 1, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )
		ENDPOINT_CHECK_BEGIN( "ep", channel_uid_wrapper_t( "" ), ne.node_uid().str(), 0 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep", "ep", channel_uid_wrapper_t( "" ), ne.node_uid().str(), 0 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	ne.remove_local_endpoint( ep );
	ne.remove_local_stagepoint( sp );

	aep = ne.utest_get_endpoint_table();
	asp = ne.utest_get_stagepoint_table();
	ci = ne.utest_get_channel_info_table();
	el = ne.query_endpoint_list();

	UT_CHECK_EQ( 0, aep.size() );
	UT_CHECK_EQ( 0, asp.size() );
	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()

	// обновляемся с каналом, все должно стать как и на первом шаге.
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

	UT_CHECK_EQ( 1, aep.size() );

	ENDPOINTS_CHECK_BEGIN( aep, el )
		ENDPOINT_CHECK_BEGIN( "ep", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
			ENDPOINT_CHECK_NO_STAGES
		ENDPOINT_CHECK_END()
	ENDPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, asp.size() );

	STAGEPOINTS_CHECK_BEGIN( asp )
		STAGEPOINT_CHECK( "ep", "ep", channel_uid_wrapper_t( "1" ), "uid-1", 1 )
	STAGEPOINTS_CHECK_END()

	UT_CHECK_EQ( 1, ci.size() );

	CHANNELS_CHECK_BEGIN( ci )
		CHANNEL_CHECK_BEGIN( channel_uid_wrapper_t( "1" ), "uid-1" )

			EXP_ENDPOINT_CHECK_BEGIN()
				EXP_ENDPOINT( "ep" )
			EXP_ENDPOINT_CHECK_END()

			EXP_STAGE_CHECK_BEGIN()
				EXP_STAGE( "ep", "ep" )
			EXP_STAGE_CHECK_END()

		CHANNEL_CHECK_END()
	CHANNELS_CHECK_END()
}
