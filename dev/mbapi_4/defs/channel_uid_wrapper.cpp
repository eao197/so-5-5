/*
	MBAPI 4.
*/

#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>

namespace mbapi_4
{

//
// channel_uid_wrapper_t
//

channel_uid_wrapper_t::channel_uid_wrapper_t()
{
}

channel_uid_wrapper_t::channel_uid_wrapper_t(
	const std::string & uid )
	:
		m_uid( uid )
{
}

channel_uid_wrapper_t::~channel_uid_wrapper_t()
{
}

} /* namespace mbapi_4 */
