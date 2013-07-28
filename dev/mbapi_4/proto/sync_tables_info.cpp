/*
	MBAPI 4.
*/

#include <mbapi_4/proto/h/sync_tables_info.hpp>


namespace mbapi_4
{

namespace proto
{

//
// endpoint_info_t
//

endpoint_info_t::endpoint_info_t()
{
}

endpoint_info_t::~endpoint_info_t()
{
}

//
// stagepoint_info_t
//

stagepoint_info_t::stagepoint_info_t()
{
}

stagepoint_info_t::~stagepoint_info_t()
{
}

//
// sync_tables_info_t
//

sync_tables_info_t::sync_tables_info_t()
	:
		req_info_t( req_id::REQ_SYNC_TABLES )
{
}

sync_tables_info_t::~sync_tables_info_t()
{
}

//
// sync_tables_info_t
//

sync_tables_resp_info_t::sync_tables_resp_info_t()
	:
		resp_info_t( req_id::RESP_SYNC_TABLES, 0 )
{
}

sync_tables_resp_info_t::~sync_tables_resp_info_t()
{
}

} /* namespace proto */

} /* namespace mbapi_4 */
