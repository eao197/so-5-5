/*
	SObjectizer 5 SOP4
*/

#include <so_5/h/exception.hpp>
#include <so_5/h/log_err.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>

#include <mbapi_4/comm/impl/h/get_layer_impl.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

mbapi_4::impl::mbapi_layer_impl_t &
get_layer_impl(
	mbapi_4::mbapi_layer_t * mbapi_layer )
{
	if( 0 == mbapi_layer )
		throw so_5::exception_t(
			"mbapi layer not set",
			rc_mbapi_layer_not_set );

	return mbapi_layer->impl();
}

} /* namespace impl */

} /* namespace comm */

} /* namespace mbapi_4 */
