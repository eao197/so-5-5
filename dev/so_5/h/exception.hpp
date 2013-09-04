/*
	SObjectizer 5.
*/

/*!
	\file
	\brief An exception class.
*/

#if !defined( _SO_5__EXCEPTION_HPP_ )
#define _SO_5__EXCEPTION_HPP_

#include <stdexcept>
#include <string>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

namespace so_5
{

//
// exception_t
//

//! The base class for all SObjectizer exceptions.
class SO_5_TYPE exception_t
	:
		public std::runtime_error
{
		//! Typedef for base type.
		typedef std::runtime_error base_type_t;
	public:
		exception_t(
			const std::string & error_descr,
			ret_code_t error_code );
		virtual ~exception_t() throw();

		//! Error code getter.
		ret_code_t
		error_code() const;

	private:
		//! Error code.
		ret_code_t m_error_code;
};

} /* namespace so_5 */

#endif

