/*
	MBAPI 4.
*/

#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/defs/h/stagepoint.hpp>

#include <mbapi_4/defs/impl/h/check_point_name.hpp>

namespace mbapi_4
{

//
// stagepoint_t
//

stagepoint_t::stagepoint_t()
{
}

stagepoint_t::stagepoint_t(
	const std::string & stagepoint_name,
	const std::string & endpoint_name )
	:
		m_stagepoint_name( impl::check_point_name( stagepoint_name ) ),
		m_endpoint_name( impl::check_point_name( endpoint_name ) )
{
}
stagepoint_t::stagepoint_t(
	const endpoint_t & endpoint )
	:
		m_stagepoint_name( endpoint.name() ),
		m_endpoint_name( endpoint.name() )
{

}

stagepoint_t::~stagepoint_t()
{
}

namespace /* ananymous */
{

const std::string g_spaces( " \t" );

std::string
trimspaces(
	const std::string & s )
{
	const size_t begin_pos = s.find_first_not_of( g_spaces );

	if( std::string::npos == begin_pos )
		return std::string(); // строка s состоит только из пробелов.

	const size_t end_pos = s.find_last_not_of( g_spaces );

	return s.substr( begin_pos, end_pos - begin_pos + 1 );
}

} /* ananymous namespace */

stagepoint_t
stagepoint_t::create_from_string(
	const std::string & s )
{
	const size_t at_pos = s.find( '@' );

	if( std::string::npos == at_pos )
	{
		const std::string & endpoint_name = trimspaces( s );
		return stagepoint_t( endpoint_name, endpoint_name );
	}

	return stagepoint_t(
		trimspaces( s.substr( 0, at_pos ) ),
		trimspaces( s.substr( at_pos + 1 ) ) );
}

} /* namespace mbapi_4 */
