/*
	MBAPI 4.
*/

#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>
#include <mbapi_4/h/mbapi_layer.hpp>
#include <mbapi_4/h/message.hpp>

namespace mbapi_4
{

//
// message_base_t
//

message_base_t::message_base_t(
	mbapi_layer_t & mbapi_layer,
	const endpoint_t & from,
	const endpoint_t & to,
	const stagepoint_t & current_stage )
	:
		m_mbapi_layer( mbapi_layer ),
		m_from( from ),
		m_to( to ),
		m_current_stage( current_stage )
{
}

message_base_t::~message_base_t()
{
}

void
message_base_t::move_next_impl(
	const oess_id_wrapper_t & oess_id,
	std::unique_ptr< oess_2::stdsn::serializable_t > msg ) const
{
	m_mbapi_layer.send(
		m_from,
		m_to,
		m_current_stage,
		oess_id,
		std::move( msg ) );
}

//
// binary_message_t
//

binary_message_t::binary_message_t(
	mbapi_layer_t & mbapi_layer,
	const endpoint_t & from,
	const endpoint_t & to,
	const stagepoint_t & current_stage,
	const oess_id_wrapper_t & oess_id_wrapper,
	const std::string & bin_message )
	:
		base_type_t(
			mbapi_layer,
			from,
			to,
			current_stage ),
		m_oess_id_wrapper( oess_id_wrapper ),
		m_bin_message( bin_message )
{
}

binary_message_t::~binary_message_t()
{
}

} /* namespace mbapi_4 */
