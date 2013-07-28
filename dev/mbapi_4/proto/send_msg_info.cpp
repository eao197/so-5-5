/*
	MBAPI 4.
*/

#include <mbapi_4/impl/h/types.hpp>

#include <mbapi_4/proto/h/send_msg_info.hpp>

namespace mbapi_4
{

namespace proto
{

//
// send_msg_info_t
//

send_msg_info_t::send_msg_info_t()
	:
		req_info_t( req_id::REQ_SEND_MSG )
{
}

send_msg_info_t::send_msg_info_t(
	comm::transmit_info_t & transmit_info )
	:
		req_info_t( req_id::REQ_SEND_MSG ),
		m_from( transmit_info.m_from.name() ),
		m_to( transmit_info.m_to.name() ),
		m_current_stage( transmit_info.m_current_stage.name() ),
		m_oess_id( transmit_info.m_oess_id.str() )
{
	if( transmit_info.m_current_stage.endpoint_name() ==
		transmit_info.m_from.name() )
		set_stage_owner_is_from();
	else
		set_stage_owner_is_to();

	m_payload.swap( transmit_info.m_payload );
}

send_msg_info_t::~send_msg_info_t()
{
}

//
// send_msg_resp_info_t
//

send_msg_resp_info_t::send_msg_resp_info_t()
	:
		resp_info_t( req_id::RESP_SEND_MSG, 0 )
{
}

send_msg_resp_info_t::send_msg_resp_info_t(
	int ret_code )
	:
		resp_info_t( req_id::RESP_SEND_MSG, ret_code )
{
}

send_msg_resp_info_t::send_msg_resp_info_t(
	int ret_code,
	const std::string & error_msg )
	:
		resp_info_t( req_id::RESP_SEND_MSG, ret_code, error_msg )
{
}

send_msg_resp_info_t::~send_msg_resp_info_t()
{
}

} /* namespace proto */

} /* namespace mbapi_4 */
