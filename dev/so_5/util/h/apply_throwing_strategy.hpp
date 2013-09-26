/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Function for applying throwing strategy to return codes.
*/

#if !defined( _SO_5__API__IMPL__APPLY_THROWING_STRATEGY_HPP_ )
#define _SO_5__API__IMPL__APPLY_THROWING_STRATEGY_HPP_

#include <string>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace util
{

/*!
	\brief Function for applying throwing strategy to return codes.

	If \a throwing_strategy is THROW_ON_ERROR then exception will be thrown.
	Otherwise \a error_code will be returned.

	\return Value of the \a error_code.
*/
SO_5_EXPORT_FUNC_SPEC( ret_code_t )
apply_throwing_strategy(
	ret_code_t error_code,
	throwing_strategy_t throwing_strategy,
	const std::string & error_msg );

/*!
	\brief Function for applying throwing strategy to exception object.

	Rethrow \a ex if \a throwing_strategy is THROW_ON_ERROR.
	Otherwise \a ex.error_code() will be returned.

	\return Value of the \a ex.error_code().
*/
SO_5_EXPORT_FUNC_SPEC( ret_code_t )
apply_throwing_strategy(
	const so_5::exception_t & ex,
	throwing_strategy_t throwing_strategy );

} /* namespace util */

} /* namespace so_5 */

#endif
