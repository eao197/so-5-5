/*
	MBAPI 4.
*/

#include <ace/UUID.h>

#include <mbapi_4/defs/h/mbapi_node_uid.hpp>

namespace mbapi_4
{

//
// mbapi_node_uid_t
//

mbapi_node_uid_t::mbapi_node_uid_t(
	const std::string & uid )
	:
		m_uid( uid )
{
}

mbapi_node_uid_t::mbapi_node_uid_t()
{
}

mbapi_node_uid_t::~mbapi_node_uid_t()
{
}

mbapi_node_uid_t
mbapi_node_uid_t::create()
{
	ACE_Utils::UUID uuid;
	ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID( uuid );

	return mbapi_node_uid_t( uuid.to_string()->c_str() );
}

mbapi_node_uid_t
mbapi_node_uid_t::utest_create_uid(
	const std::string & uid )
{
	return mbapi_node_uid_t( uid );
}

} /* namespace mbapi_4 */
