/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Функция для сравнения указателей
	на методы обработчики событий.
*/

#if !defined( _SO_5__RT__IMPL__CMP_METHOD_PTR_HPP_ )
#define _SO_5__RT__IMPL__CMP_METHOD_PTR_HPP_

#include <cstring>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/event_data.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// cmp_method_ptr
//

//! Сравнить указатели на метод класса агента.
/*!
	\return true если указатели на методы равны,
	false если указатели на методы не равны.
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
