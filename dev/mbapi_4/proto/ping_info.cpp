/*
	SObjectizer 5 SOP4
*/

#include <oess_2/stdsn/h/inout_templ.hpp>

#include <mbapi_4/proto/h/ping_info.hpp>

namespace mbapi_4
{

namespace proto
{

//
// ping_info_t
//

ping_info_t::ping_info_t()
	:
		req_info_t( req_id::REQ_PING )
{
}

ping_info_t::~ping_info_t()
{
}

//
// ping_resp_info_t
//

ping_resp_info_t::ping_resp_info_t()
	:
		resp_info_t( req_id::RESP_PING, 0 )
{
}

ping_resp_info_t::~ping_resp_info_t()
{
}

} /* namespace proto */

} /* namespace mbapi_4 */
