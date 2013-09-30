/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Wrapper for the std::type_info.
*/

#if !defined( _SO_5__RT__TYPE_WRAPPER_HPP_ )
#define _SO_5__RT__TYPE_WRAPPER_HPP_

#include <typeinfo>
#include <cstring>

namespace so_5
{

namespace rt
{

//
// type_wrapper_t
//

/*!
	Wrapper class for the std::type_info which adds 
	compare operators to it.

	Also used for implementing layers feature.
*/
class type_wrapper_t
{
	public:
		inline
		type_wrapper_t(
			//! Message type.
			const std::type_info & type_info )
			:	m_type_info( &type_info )
		{}

		//! The less operator which makes it possible to
		//! use this class as a std::map key.
		/*! 
			The comparison uses m_type_info of objects.
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
		//! Runtime type info.
		const std::type_info * m_type_info;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
