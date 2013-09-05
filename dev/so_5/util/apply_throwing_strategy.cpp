/*
	SObjectizer 5.
*/

#include <so_5/util/h/apply_throwing_strategy.hpp>
#include <so_5/h/exception.hpp>

namespace so_5
{

namespace util
{

SO_5_EXPORT_FUNC_SPEC( ret_code_t )
apply_throwing_strategy(
	ret_code_t error_code,
	throwing_strategy_t throwing_strategy,
	const std::string & error_msg )
{
	if( THROW_ON_ERROR == throwing_strategy )
		throw so_5::exception_t( error_msg, error_code );

	return error_code;
}

SO_5_EXPORT_FUNC_SPEC( ret_code_t )
apply_throwing_strategy(
	const so_5::exception_t & ex,
	throwing_strategy_t throwing_strategy )
{
	if( THROW_ON_ERROR == throwing_strategy )
		throw so_5::exception_t( ex.what(), ex.error_code() );

	return ex.error_code();
}

} /* namespace util */

} /* namespace so_5 */

