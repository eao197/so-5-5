/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_handler_caller_ref.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>

namespace so_5
{

namespace rt
{

event_handler_caller_ref_t::event_handler_caller_ref_t(
	event_handler_caller_t * event_handler_caller_ptr )
	:
		m_event_handler_caller_ptr( event_handler_caller_ptr )
{
	inc_event_handler_caller_ref_count();
}

event_handler_caller_ref_t::event_handler_caller_ref_t()
	:
		m_event_handler_caller_ptr( nullptr )
{
}

event_handler_caller_ref_t::event_handler_caller_ref_t(
	const event_handler_caller_ref_t &
		event_handler_caller_ref )
	:
		m_event_handler_caller_ptr(
			event_handler_caller_ref.m_event_handler_caller_ptr )
{
	inc_event_handler_caller_ref_count();
}

void
event_handler_caller_ref_t::operator = (
	const event_handler_caller_ref_t &
		event_handler_caller_ref )
{
	if( &event_handler_caller_ref != this )
	{
		dec_event_handler_caller_ref_count();

		m_event_handler_caller_ptr =
			event_handler_caller_ref.m_event_handler_caller_ptr;

		inc_event_handler_caller_ref_count();
	}
}

event_handler_caller_ref_t::~event_handler_caller_ref_t()
{
	dec_event_handler_caller_ref_count();
}

event_handler_caller_t *
event_handler_caller_ref_t::get() const
{
	return m_event_handler_caller_ptr;
}

event_handler_caller_t *
event_handler_caller_ref_t::operator -> ()
{
	return m_event_handler_caller_ptr;
}

const event_handler_caller_t *
event_handler_caller_ref_t::operator -> () const
{
	return m_event_handler_caller_ptr;
}

event_handler_caller_t &
event_handler_caller_ref_t::operator * ()
{
	return *m_event_handler_caller_ptr;
}

const event_handler_caller_t &
event_handler_caller_ref_t::operator * () const
{
	return *m_event_handler_caller_ptr;
}

void
event_handler_caller_ref_t::release()
{
	dec_event_handler_caller_ref_count();
	m_event_handler_caller_ptr = nullptr;
}

bool
event_handler_caller_ref_t::operator == (
	const event_handler_caller_ref_t &
		event_handler_caller_ref ) const
{
	if( m_event_handler_caller_ptr->target_state() ==
		event_handler_caller_ref->target_state() )
		return
			m_event_handler_caller_ptr->type_wrapper() ==
				event_handler_caller_ref->type_wrapper();

	return false;
}


inline void
event_handler_caller_ref_t::dec_event_handler_caller_ref_count()
{
	if( m_event_handler_caller_ptr &&
		0 == m_event_handler_caller_ptr->dec_ref_count() )
	{
		delete m_event_handler_caller_ptr;
		m_event_handler_caller_ptr = nullptr;
	}
}

inline void
event_handler_caller_ref_t::inc_event_handler_caller_ref_count()
{
	if( m_event_handler_caller_ptr )
		m_event_handler_caller_ptr->inc_ref_count();
}

} /* namespace rt */

} /* namespace so_5 */
