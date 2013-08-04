/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Wrapper for std::type_info.
*/

#if !defined( _SO_5__RT__TYPE_WRAPPER_HPP_ )
#define _SO_5__RT__TYPE_WRAPPER_HPP_

#include <typeinfo>
#include <cstring>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

//
// type_wrapper_t
//

/*!
	Wrapper class for std::type_info that adds 
	compare operators to it.

	Also used for implementing layers feature.
*/
class SO_5_TYPE type_wrapper_t
{
	public:
		type_wrapper_t(
			//! Message type.
			const std::type_info & type_info );

		type_wrapper_t &
		operator = (
			const type_wrapper_t & type_wrapper );

		~type_wrapper_t();

		//! The less operator, that makes it possible to
		//! use the class as std::map key.
		/*! 
			Comparison uses m_type_info of objects.
		*/
		inline bool
		operator < (
			const type_wrapper_t & typeid_wrapper ) const
		{
			return std::strcmp(
				m_type_info->name(),
				typeid_wrapper.m_type_info->name() ) < 0;
		}

		inline bool
		operator == (
			const type_wrapper_t & typeid_wrapper ) const
		{
			return *m_type_info == *typeid_wrapper.m_type_info;
		}

		//! Access type_info.
		inline const std::type_info &
		query_type_info() const
		{
			return *m_type_info;
		}

	private:
		//! Runtime typy info.
		const std::type_info * m_type_info;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
