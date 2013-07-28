/*
	MBAPI 4.
*/

#include <mbapi_4/h/oess_id_wrapper.hpp>

namespace mbapi_4
{

oess_id_wrapper_t::oess_id_wrapper_t()
{
}

oess_id_wrapper_t::oess_id_wrapper_t(
	const std::string & type_name )
	:
		m_type_name( type_name )
{
}

oess_id_wrapper_t::~oess_id_wrapper_t()
{
}

} /* namespace mbapi_4 */
