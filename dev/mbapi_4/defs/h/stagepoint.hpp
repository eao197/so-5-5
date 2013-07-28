/*
	MBAPI 4.
*/

/*!
	\file
	\brief Промежуточная стадия.
*/

#if !defined( _MBAPI_4__STAGEPOINT_HPP_ )
#define _MBAPI_4__STAGEPOINT_HPP_

#include <iosfwd>

#include <string>
#include <vector>

#include <mbapi_4/defs/h/declspec.hpp>

namespace mbapi_4
{

class endpoint_t;

//
// stagepoint_t
//

//! Промежуточная стадия конечной точки.
/*!
	Точка стадия служит, как для представления точки-стадии,
	так и для представление самой конечной точки, в последнем случае
	имя сталии совпадает с именем конечной точки.

	Имя стадии может состоять только из:
	\li букв английского алфавита a..zA..Z;
	\li цифр 0..9;
	\li символов "_-./|#$".
*/
class MBAPI_4_DEFS_TYPE stagepoint_t
{
	public:
		stagepoint_t();

		stagepoint_t(
			//! Имя стадии.
			const std::string & stagepoint_name,
			//! Имя конечной точки.
			const std::string & endpoint_name );

		//! Создает стадию, которая соответствует самой конечной точке.
		stagepoint_t(
			//! Имя стадии.
			const endpoint_t & endpoint );

		~stagepoint_t();

		//! Создать стадию из строки.
		/*!
			Формат строки должен быть следующий:
				\li stagename@endpointname;
				\li endpointname.

			В первом случае описывается промежуточная стадия,
			во втором стадия, которая представляет собой
			саму конечную точку.
		*/
		static stagepoint_t
		create_from_string(
			const std::string & s );

		//! Операторы сравнения.
		//! \{
		inline bool
		operator < ( const stagepoint_t & stagepoint ) const
		{
			if( m_endpoint_name != stagepoint.m_endpoint_name )
				return m_endpoint_name < stagepoint.m_endpoint_name;

			return m_stagepoint_name < stagepoint.m_stagepoint_name;
		}

		inline bool
		operator == ( const stagepoint_t & stagepoint ) const
		{
			return
				m_endpoint_name == stagepoint.m_endpoint_name &&
				m_stagepoint_name == stagepoint.m_stagepoint_name;
		}
		//! \}

		//! Проверить является ли стадия конечной, т.е.
		//! конечной точкой.
		inline bool
		is_endpoint() const
		{
			return m_stagepoint_name == m_endpoint_name;
		}

		//! Получение и изменение имен стадии и конечной точки.
		//! \{
		inline const std::string &
		name() const
		{
			return m_stagepoint_name;
		}

		inline void
		set_name(
			const std::string & name )
		{
			m_stagepoint_name = name;
		}

		inline const std::string &
		endpoint_name() const
		{
			return m_endpoint_name;
		}

		//! Изменить имя конечной точки.
		inline void
		set_endpoint_name(
			const std::string & endpoint_name )
		{
			m_endpoint_name = endpoint_name;
		}
		//! \}


	private:
		//! Имя стадии.
		std::string m_stagepoint_name;

		//! Имя конечной точки.
		std::string m_endpoint_name;
};

} /* namespace mbapi_4 */

#endif
