/*
	SObjectizer 5.
*/

#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace rt
{

mbox_t::mbox_t()
{
}

mbox_t::~mbox_t()
{
}

bool
mbox_t::operator==( const mbox_t & o ) const
{
	return cmp_ordinal() == o.cmp_ordinal();
}

bool
mbox_t::operator<( const mbox_t & o ) const
{
	return cmp_ordinal() < o.cmp_ordinal();
}

const mbox_t *
mbox_t::cmp_ordinal() const
{
	return this;
}

} /* namespace rt */

} /* namespace so_5 */
