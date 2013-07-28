/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/channel_controller.hpp>

namespace so_5_transport
{

//
// channel_controller_t
//

channel_controller_t::channel_controller_t()
{
}

channel_controller_t::~channel_controller_t()
{
}

//
// channel_controller_ref_t
//

channel_controller_ref_t::channel_controller_ref_t()
	:
		m_channel_controller_ptr( nullptr )
{
}

channel_controller_ref_t::channel_controller_ref_t(
	channel_controller_t * channel_controller )
	:
		m_channel_controller_ptr( channel_controller )
{
	inc_channel_controller_ref_count();
}

channel_controller_ref_t::channel_controller_ref_t(
	const channel_controller_ref_t & channel_controller_ref )
	:
		m_channel_controller_ptr( channel_controller_ref.m_channel_controller_ptr )
{
	inc_channel_controller_ref_count();
}

void
channel_controller_ref_t::operator = (
	const channel_controller_ref_t & channel_controller_ref )
{
	if( &channel_controller_ref != this )
	{
		dec_channel_controller_ref_count();

		m_channel_controller_ptr = channel_controller_ref.m_channel_controller_ptr;
		inc_channel_controller_ref_count();
	}

}

channel_controller_ref_t::~channel_controller_ref_t()
{
	dec_channel_controller_ref_count();
}

inline void
channel_controller_ref_t::dec_channel_controller_ref_count()
{
	if( m_channel_controller_ptr &&
		0 == m_channel_controller_ptr->dec_ref_count() )
	{
		delete m_channel_controller_ptr;
		m_channel_controller_ptr = nullptr;
	}
}

inline void
channel_controller_ref_t::inc_channel_controller_ref_count()
{
	if( m_channel_controller_ptr )
		m_channel_controller_ptr->inc_ref_count();
}

} /* namespace so_5_transport */

