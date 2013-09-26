/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A function to compare pointers to methods.
*/

#if !defined( _SO_5__RT__IMPL__CMP_METHOD_PTR_HPP_ )
#define _SO_5__RT__IMPL__CMP_METHOD_PTR_HPP_

#include <cstring>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// cmp_method_ptr
//

//! A function to compare pointers to methods.
/*!
	\retval true pointers are equal.
	\retval true pointers are not equal.
*/

inline bool
cmp_method_ptr(
	const char * left_ordinal,
	size_t left_size,
	const char * right_ordinal,
	size_t right_size )
{
	return left_size == right_size &&
		0 == memcmp(
			left_ordinal,
			right_ordinal,
			left_size );
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
