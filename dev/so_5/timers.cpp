/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.5.0
	\brief Timers and tools for working with timers.
*/

#include <so_5/h/timers.hpp>

namespace so_5
{

//
// timer_t
//
timer_t::~timer_t()
	{}

//
// timer_id_t
//
timer_id_t::timer_id_t()
	{}

timer_id_t::timer_id_t(
	so_5::rt::smart_atomic_reference_t< timer_t > && timer )
	:	m_timer( std::move( timer ) )
	{}

timer_id_t::timer_id_t(
	const timer_id_t & o )
	:	m_timer( o.m_timer )
	{}

timer_id_t::timer_id_t(
	timer_id_t && o )
	:	m_timer( std::move( o.m_timer ) )
	{}

timer_id_t::~timer_id_t()
	{}

timer_id_t &
timer_id_t::operator=( const timer_id_t & o )
	{
		timer_id_t t( o );
		t.swap( *this );
		return *this;
	}

timer_id_t &
timer_id_t::operator=( timer_id_t && o )
	{
		timer_id_t t( std::move( o ) );
		t.swap( *this );
		return *this;
	}

void
timer_id_t::swap( timer_id_t & o )
	{
		m_timer.swap( o.m_timer );
	}

bool
timer_id_t::is_active() const
	{
		return ( m_timer && m_timer->is_active() );
	}

void
timer_id_t::release()
	{
		if( m_timer )
			m_timer->release();
	}

} /* namespace so_5 */

