/*
	SObjectizer 5.
*/

#include <so_5/rt/h/message_ref.hpp>
#include <so_5/rt/h/message.hpp>

namespace so_5
{

namespace rt
{

message_ref_t::message_ref_t()
	:
		m_message_ptr( nullptr )
{
}

message_ref_t::message_ref_t(
	message_t * message )
	:
		m_message_ptr( message )
{
	inc_message_ref_count();
}

message_ref_t::message_ref_t(
	const message_ref_t & message_ref )
	:
		m_message_ptr( message_ref.m_message_ptr )
{
	inc_message_ref_count();
}

void
message_ref_t::operator = ( const message_ref_t & message_ref )
{
	if( this != &message_ref )
	{
		dec_message_ref_count();

		m_message_ptr = message_ref.m_message_ptr;
		inc_message_ref_count();
	}
}

message_ref_t::~message_ref_t()
{
	dec_message_ref_count();
}

inline void
message_ref_t::inc_message_ref_count()
{
	if( m_message_ptr )
		m_message_ptr->inc_ref_count();
}

inline void
message_ref_t::dec_message_ref_count()
{
	if( m_message_ptr &&
		0 == m_message_ptr->dec_ref_count() )
	{
		delete m_message_ptr;
		m_message_ptr = nullptr;
	}
}

} /* namespace rt */

} /* namespace so_5 */
