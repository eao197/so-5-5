/*
	SObjectizer 5.
*/

#include <so_5/h/exception.hpp>


namespace so_5
{

//
// exception_t
//

exception_t::exception_t(
	const std::string & error_descr,
	ret_code_t error_code )
	:
		base_type_t( error_descr ),
		m_error_code( error_code )
{
}

exception_t::~exception_t() throw()
{
}

ret_code_t
exception_t::error_code() const
{
	return m_error_code;
}

} /* namespace so_5 */
