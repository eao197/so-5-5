/*
	MBAPI 4.
*/

/*!
	\file
	\brief Функция для проверки имени конечной точи и стадии.
*/

#if !defined( _MBAPI_4__DEFS__IMPL__CHECK_POINT_NAME_HPP_ )
#define _MBAPI_4__DEFS__IMPL__CHECK_POINT_NAME_HPP_

#include <string>

namespace mbapi_4
{

namespace impl
{

//
// check_point_name
//

//! Функция проверки имени точки.
/*!
	Если имя точки соответствует формату,
	то фукнция отрабатывает без исключений,
	в противном случае, выбрасывается исключение.

	Имя конечной точки может состоять только из:
	\li букв английского алфавита a..zA..Z;
	\li цифр 0..9;
	\li символов "_-|#$".
*/

const std::string &
check_point_name(
	const std::string & point_name );

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
