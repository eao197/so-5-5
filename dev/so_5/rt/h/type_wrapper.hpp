/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Обертка на std::type_info.
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
	Вспомогательный класс - обертка на std::type_info типов сообщений,
	который определяет оператор сравнения.

	Также используется инфраструктурой слоев SObjectizer.
*/
class SO_5_TYPE type_wrapper_t
{
	public:
		type_wrapper_t(
			//! Информация о типе сообщения.
			const std::type_info & type_info );

		type_wrapper_t &
		operator = (
			const type_wrapper_t & type_wrapper );

		~type_wrapper_t();

		//! Оператор сравнения для использования
		//! в качестве ключа в std::map.
		/*! Cравнивает std::type_info *,
			которые храняться в m_type_info.
		*/
		inline bool
		operator < (
			const type_wrapper_t & typeid_wrapper ) const
		{
			return std::strcmp(
				m_type_info->name(),
				typeid_wrapper.m_type_info->name() ) < 0;
		}

		bool
		operator == (
			const type_wrapper_t & typeid_wrapper ) const
		{
			return *m_type_info == *typeid_wrapper.m_type_info;
		}

		//! Получить информацию о типе сообщения
		const std::type_info &
		query_type_info() const;

	private:
		//! Информация о типе сообщения.
		/*!
			При сравнении различных типов, под обертками
			они сравниваются через адреса их std::type_info,
			т.е. m_type_info.
		*/
		const std::type_info * m_type_info;
};

//
// quick_typeid_t
//

//! Класс для оптимизации получения обертки на тип сообщения из
//! шаблонных методов.
template <class MESSAGE >
class quick_typeid_t
{
	public:
		//! Обертка на тип сообщения.
		static const type_wrapper_t m_type_wrapper;
};

// Инициализация обертки, которая происходит 1 раз.
template <class MESSAGE >
const type_wrapper_t
	quick_typeid_t< MESSAGE >::m_type_wrapper =
		type_wrapper_t( typeid( MESSAGE ) );

} /* namespace rt */

} /* namespace so_5 */

#endif
