/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс nonempty_name_t.
*/

#if !defined( _SO_5__RT__NONEMPTY_NAME_HPP_ )
#define _SO_5__RT__NONEMPTY_NAME_HPP_

#include <string>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

//
// nonempty_name_t
//

//! Класс для инкапсуляции имени, которое не может быть пустым.
/*!
	Если в конструктор объекта передается пустая строка,
	то будет выброшено исключение.
*/
class SO_5_TYPE nonempty_name_t
{
	public:
		//! Имя не должно быть пустым.
		nonempty_name_t(
			const char * name );

		//! Имя не должно быть пустым.
		nonempty_name_t(
			const std::string & name );

		~nonempty_name_t();

		//! Получить имя.
		inline const std::string &
		query_name() const
		{
			return m_nonempty_name;
		}

	private:
		//! Имя.
		std::string m_nonempty_name;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
