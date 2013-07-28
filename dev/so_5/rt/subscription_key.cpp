/*
	SObjectizer 5.
*/

#include <so_5/rt/h/subscription_key.hpp>

namespace so_5
{

namespace rt
{

subscription_key_t::subscription_key_t(
	const type_wrapper_t & type_wrapper,
	const mbox_ref_t & mbox )
	:
		m_type_wrapper( type_wrapper ),
		m_mbox( mbox )
{
}

subscription_key_t::~subscription_key_t()
{
}

bool
subscription_key_t::operator < (
	const subscription_key_t & subscription_key ) const
{
	if( subscription_key.m_type_wrapper ==
		m_type_wrapper )
	{
		return m_mbox < subscription_key.m_mbox;
	}

	return m_type_wrapper <
		subscription_key.m_type_wrapper;
}

bool
subscription_key_t::operator == (
	const subscription_key_t & subscription_key ) const
{
	return
		subscription_key.m_type_wrapper ==
			m_type_wrapper &&
		subscription_key.m_mbox == m_mbox;
}

} /* namespace rt */

} /* namespace so_5 */
