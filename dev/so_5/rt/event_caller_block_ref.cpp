/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_caller_block_ref.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

namespace so_5
{

namespace rt
{

event_caller_block_ref_t::event_caller_block_ref_t(
	event_caller_block_t * event_caller_block_ptr )
	:
		m_event_caller_block_ptr( event_caller_block_ptr )
{
	inc_event_caller_block_ref_count();
}

event_caller_block_ref_t::event_caller_block_ref_t()
	:
		m_event_caller_block_ptr( nullptr )
{
}

event_caller_block_ref_t::event_caller_block_ref_t(
	const event_caller_block_ref_t &
		event_caller_block_ref )
	:
		m_event_caller_block_ptr(
			event_caller_block_ref.m_event_caller_block_ptr )
{
	inc_event_caller_block_ref_count();
}

void
event_caller_block_ref_t::operator = (
	const event_caller_block_ref_t &
		event_caller_block_ref )
{
	if( &event_caller_block_ref != this )
	{
		dec_event_caller_block_ref_count();

		m_event_caller_block_ptr =
			event_caller_block_ref.m_event_caller_block_ptr;

		inc_event_caller_block_ref_count();
	}
}

event_caller_block_ref_t::~event_caller_block_ref_t()
{
	dec_event_caller_block_ref_count();
}

event_caller_block_t *
event_caller_block_ref_t::get() const
{
	return m_event_caller_block_ptr;
}

event_caller_block_t *
event_caller_block_ref_t::operator -> ()
{
	return m_event_caller_block_ptr;
}

const event_caller_block_t *
event_caller_block_ref_t::operator -> () const
{
	return m_event_caller_block_ptr;
}

event_caller_block_t &
event_caller_block_ref_t::operator * ()
{
	return *m_event_caller_block_ptr;
}

const event_caller_block_t &
event_caller_block_ref_t::operator * () const
{
	return *m_event_caller_block_ptr;
}

void
event_caller_block_ref_t::release()
{
	dec_event_caller_block_ref_count();
	m_event_caller_block_ptr = nullptr;
}

inline void
event_caller_block_ref_t::dec_event_caller_block_ref_count()
{
	if( m_event_caller_block_ptr &&
		0 == m_event_caller_block_ptr->dec_ref_count() )
	{
		delete m_event_caller_block_ptr;
		m_event_caller_block_ptr = nullptr;
	}
}

inline void
event_caller_block_ref_t::inc_event_caller_block_ref_count()
{
	if( m_event_caller_block_ptr )
		m_event_caller_block_ptr->inc_ref_count();
}

} /* namespace rt */

} /* namespace so_5 */

