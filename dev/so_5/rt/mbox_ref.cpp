/*
	SObjectizer 5.
*/

#include <iostream>

#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace rt
{

mbox_ref_t::mbox_ref_t()
	:
		m_mbox_ptr( nullptr )
{
}

mbox_ref_t::mbox_ref_t(
	mbox_t * mbox )
	:
		m_mbox_ptr( mbox )
{
	inc_mbox_ref_count();
}

mbox_ref_t::mbox_ref_t(
	const mbox_ref_t & mbox_ref )
	:
		m_mbox_ptr( mbox_ref.m_mbox_ptr )
{
	inc_mbox_ref_count();
}

mbox_ref_t &
mbox_ref_t::operator = ( const mbox_ref_t & mbox_ref )
{
	if( &mbox_ref != this )
	{
		dec_mbox_ref_count();

		m_mbox_ptr = mbox_ref.m_mbox_ptr;
		inc_mbox_ref_count();
	}

	return *this;
}

mbox_ref_t::~mbox_ref_t()
{
	dec_mbox_ref_count();
}

void
mbox_ref_t::release()
{
	dec_mbox_ref_count();
	m_mbox_ptr = nullptr;
}

bool
mbox_ref_t::operator == ( const mbox_ref_t & mbox_ref ) const
{
	const mbox_t * right =
		m_mbox_ptr ? m_mbox_ptr->cmp_ordinal() : nullptr;
	const mbox_t * left =
		mbox_ref.m_mbox_ptr ? mbox_ref.m_mbox_ptr->cmp_ordinal() : nullptr;

	return right == left;
}

bool
mbox_ref_t::operator < ( const mbox_ref_t & mbox_ref ) const
{
	const mbox_t * right =
		m_mbox_ptr ? m_mbox_ptr->cmp_ordinal() : nullptr;
	const mbox_t * left =
		mbox_ref.m_mbox_ptr ? mbox_ref.m_mbox_ptr->cmp_ordinal() : nullptr;

	return right < left;
}

inline void
mbox_ref_t::dec_mbox_ref_count()
{
	if( m_mbox_ptr &&
		0 == m_mbox_ptr->dec_ref_count() )
	{
		delete m_mbox_ptr;
		m_mbox_ptr = nullptr;
	}
}

inline void
mbox_ref_t::inc_mbox_ref_count()
{
	if( m_mbox_ptr )
		m_mbox_ptr->inc_ref_count();
}

} /* namespace rt */

} /* namespace so_5 */
