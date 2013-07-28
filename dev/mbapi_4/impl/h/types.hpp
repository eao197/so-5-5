/*
	MBAPI 4.
*/

/*!
	\file
	\brief Вспомогательные определения типов.
*/

#if !defined( _MBAPI_4__IMPL__TYPES_HPP_ )
#define _MBAPI_4__IMPL__TYPES_HPP_

#include <vector>

#include <oess_2/defs/h/types.hpp>

#include <oess_2/stdsn/h/serializable.hpp>

namespace mbapi_4
{

namespace impl
{

//! Тип для списка репозиториев.
typedef std::vector< const oess_2::stdsn::types_repository_t * >
	types_repositories_list_t;

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
