/*
	SObjectizer 5.
*/

#include <so_5/rt/h/message.hpp>

namespace so_5
{

namespace rt
{

//
// message_t
//

message_t::message_t()
{
}

message_t::message_t( const message_t & )
{
}

message_t::~message_t()
{
}

void
message_t::operator = ( const message_t & )
{
}

bool
message_t::check() const
{
	// По умолчанию проверка проходит.
	return true;
}

} /* namespace rt */

} /* namespace so_5 */
