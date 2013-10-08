/*
	SObjectizer 5.
*/

#include <so_5/timer_thread/h/timer_id.hpp>
#include <so_5/timer_thread/h/timer_thread.hpp>

namespace so_5
{

namespace timer_thread
{

//
// timer_id_internals_t
//

timer_id_internals_t::timer_id_internals_t(
	timer_thread_t & timer_thread,
	timer_id_t timer_id )
	:
		m_timer_thread( timer_thread ),
		m_timer_id( timer_id )
{
}

timer_id_internals_t::~timer_id_internals_t()
{
	m_timer_thread.cancel_act( m_timer_id );
}

//
// timer_id_ref_t
//

timer_id_ref_t::timer_id_ref_t(
	timer_id_internals_t * timer_id_internals )
	:
		m_timer_id_internals( timer_id_internals )
{
	inc_timer_id_ref_count();
}

timer_id_ref_t
timer_id_ref_t::create(
	timer_thread_t & timer_thread,
	timer_id_t timer_id )
{
	return timer_id_ref_t(
		new timer_id_internals_t( timer_thread, timer_id ) );
}

timer_id_ref_t::timer_id_ref_t()
	:
		m_timer_id_internals( nullptr )
{
}

timer_id_ref_t::timer_id_ref_t(
	const timer_id_ref_t & timer_id )
	:
		m_timer_id_internals( timer_id.m_timer_id_internals )
{
	inc_timer_id_ref_count();
}

timer_id_ref_t::timer_id_ref_t(
	timer_id_ref_t && timer_id )
	:
		m_timer_id_internals( timer_id.release_ownership() )
{
}

timer_id_ref_t::~timer_id_ref_t()
{
	dec_timer_id_ref_count();
}

timer_id_ref_t &
timer_id_ref_t::operator=( const timer_id_ref_t & timer_id )
{
	if( &timer_id != this )
	{
		dec_timer_id_ref_count();
		m_timer_id_internals = timer_id.m_timer_id_internals;
		inc_timer_id_ref_count();
	}

	return *this;
}

timer_id_ref_t &
timer_id_ref_t::operator=( timer_id_ref_t && timer_id )
{
	if( &timer_id != this )
	{
		release();
		m_timer_id_internals = timer_id.release_ownership();
	}

	return *this;
}

bool
timer_id_ref_t::is_active() const
{
	return nullptr != m_timer_id_internals;
}

void
timer_id_ref_t::release()
{
	dec_timer_id_ref_count();
	m_timer_id_internals = nullptr;
}

void
timer_id_ref_t::inc_timer_id_ref_count()
{
	if( m_timer_id_internals )
		m_timer_id_internals->inc_ref_count();
}

void
timer_id_ref_t::dec_timer_id_ref_count()
{
	if( m_timer_id_internals &&
		0 == m_timer_id_internals->dec_ref_count() )
	{
		delete m_timer_id_internals;
		m_timer_id_internals = nullptr;
	}
}

timer_id_internals_t *
timer_id_ref_t::release_ownership()
{
	auto r = m_timer_id_internals;
	m_timer_id_internals = nullptr;

	return r;
}

} /* namespace timer_thread */

} /* namespace so_5 */
