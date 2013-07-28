/*
	MBAPI 4.
*/

#include <oess_2/stdsn/h/inout_templ.hpp>

#include <mbapi_4/proto/h/req_info.hpp>

namespace mbapi_4
{

namespace proto
{

//
// req_info_t
//

req_info_t::req_info_t()
	:
		m_req_id( 0 )
{
}

req_info_t::req_info_t( oess_2::int_t req_id )
	:
		m_req_id( req_id )
{
}

req_info_t::~req_info_t()
{
}

oess_2::int_t
req_info_t::query_req_id() const
{
	return m_req_id;
}

//
// resp_info_t
//

resp_info_t::resp_info_t()
	:
		m_ret_code( 0 )
{
}

resp_info_t::resp_info_t(
	oess_2::int_t req_id,
	oess_2::int_t ret_code )
	:
		req_info_t( req_id ),
		m_ret_code( ret_code )
{
}

resp_info_t::resp_info_t(
	oess_2::int_t req_id,
	oess_2::int_t ret_code,
	const std::string & error_msg )
	:
		req_info_t( req_id ),
		m_ret_code( ret_code ),
		m_error_msg( error_msg )
{
}

resp_info_t::~resp_info_t()
{
}

oess_2::int_t
resp_info_t::query_ret_code() const
{
	return m_ret_code;
}

const std::string &
resp_info_t::query_error_msg() const
{
	return m_error_msg;
}

//
// unknown_resp_info_t
//

unknown_resp_info_t::unknown_resp_info_t()
	:
		resp_info_t( req_id::RESP_UNKNOWN, 0 )
{
}

unknown_resp_info_t::unknown_resp_info_t(
	oess_2::int_t ret_code,
	const std::string & error_msg )
	:
		resp_info_t( req_id::RESP_UNKNOWN, ret_code, error_msg )
{
}

unknown_resp_info_t::~unknown_resp_info_t() {
}

} /* namespace proto */

} /* namespace mbapi_4 */
