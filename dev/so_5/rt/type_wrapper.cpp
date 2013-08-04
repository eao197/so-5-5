/*
	SObjectizer 5.
*/

#include <string.h>

#include <so_5/rt/h/type_wrapper.hpp>

namespace so_5
{

namespace rt
{

//
// type_wrapper_t
//

type_wrapper_t::type_wrapper_t(
	const std::type_info & type_info )
	:
		m_type_info( &type_info )
{
}

type_wrapper_t::~type_wrapper_t()
{
}

type_wrapper_t &
type_wrapper_t::operator = (
	const type_wrapper_t & type_wrapper )
{
	if( &type_wrapper != this )
	{
		m_type_info = type_wrapper.m_type_info;
	}

	return *this;
}

} /* namespace rt */

} /* namespace so_5 */
