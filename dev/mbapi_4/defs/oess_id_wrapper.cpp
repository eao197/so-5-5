/*
	MBAPI 4.
*/

#include <mbapi_4/defs/h/oess_id_wrapper.hpp>

namespace mbapi_4
{

oess_id_wrapper_t::oess_id_wrapper_t()
{
}

oess_id_wrapper_t::oess_id_wrapper_t(
	const std::string & type_name )
	:
		m_local_storrage( new std::string( type_name ) ),
		m_type_name( m_local_storrage.get() )
{
}

oess_id_wrapper_t::oess_id_wrapper_t(
	const std::string * type_name )
	:
		m_type_name( type_name )
{
}

oess_id_wrapper_t::oess_id_wrapper_t(
	const oess_id_wrapper_t & oess_id )
{
	if( 0 != oess_id.m_local_storrage.get() )
	{
		m_local_storrage.reset( new std::string( oess_id.str() ) );
		m_type_name = m_local_storrage.get();
	}
	else
		m_type_name = oess_id.m_type_name;
}

void
oess_id_wrapper_t::operator = (
	const oess_id_wrapper_t & oess_id )
{
	if( 0 != oess_id.m_local_storrage.get() )
	{
		m_local_storrage.reset( new std::string( oess_id.str() ) );
		m_type_name = m_local_storrage.get();
	}
	else
	{
		m_type_name = oess_id.m_type_name;
		m_local_storrage.reset();
	}
}

oess_id_wrapper_t::~oess_id_wrapper_t()
{
}

} /* namespace mbapi_4 */
