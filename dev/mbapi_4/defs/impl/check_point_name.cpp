/*
	MBAPI 4.
*/

#include <so_5/h/exception.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/defs/impl/h/check_point_name.hpp>

namespace mbapi_4
{

namespace impl
{

namespace /* ananymous */
{

//! Проверяет является ли символ одним из "_-@#$".
bool
is_exrta_valid_symbol( char c )
{
	return
		'_' == c ||
		'-' == c ||
		'.' == c ||
		'/' == c ||
		'|' == c ||
		'#' == c ||
		'$' == c;
}


bool
valid_point_name_symbol(
	char c )
{
	return
		'a' <= c && c <= 'z' ||
		'A' <= c && c <= 'Z' ||
		'0' <= c && c <= '9' ||
		is_exrta_valid_symbol( c );
}

} /* ananymous namespace */


//
// check_point_name
//

const std::string &
check_point_name(
	const std::string & point_name )
{
	if( point_name.empty() )
		throw so_5::exception_t(
			"point name cannot be empty",
			rc_invalid_point_name );

	for(
		std::string::const_iterator
			it = point_name.begin(),
			it_end = point_name.end();
		it != it_end;
		++it )
	{
		if( !valid_point_name_symbol( *it ) )
		{
			throw so_5::exception_t(
				"invalid symbol '" + std::string( 1, *it ) +
				"' in point name: " + point_name,
				rc_invalid_point_name );
		}
	}

	return point_name;
}

} /* namespace impl */

} /* namespace mbapi_4 */