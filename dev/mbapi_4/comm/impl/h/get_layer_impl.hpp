/*
	SObjectizer 5 SOP4
*/

/*!
	\file
	\brief Вспомогательная функция для получения mbapi_layer_impl_t.
*/

#if !defined( _MBAPI_4__COMM__IMPL__GET_LAYER_IMPL_HPP_ )
#define _MBAPI_4__COMM__IMPL__GET_LAYER_IMPL_HPP_

#include <mbapi_4/h/mbapi_layer.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

mbapi_4::impl::mbapi_layer_impl_t &
get_layer_impl(
	mbapi_4::mbapi_layer_t * mbapi_layer );

} /* namespace impl */

} /* namespace comm */

} /* namespace mbapi_4 */

#endif
