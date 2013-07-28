/*
	MBAPI 4.
*/

/*!
	\file
	\brief Конечная точка для приема и отправки мообщений.
*/

#if !defined( _MBAPI_4__ENDPOINT_HPP_ )
#define _MBAPI_4__ENDPOINT_HPP_

#include <iosfwd>
#include <string>
#include <vector>

#include <mbapi_4/defs/h/declspec.hpp>

namespace mbapi_4
{

//
// endpoint_t
//

//! Конечная точка.
/*!
	Имя конечной точки может состоять только из:
	\li букв английского алфавита a..zA..Z;
	\li цифр 0..9;
	\li символов "_-./|#$".
*/
class MBAPI_4_DEFS_TYPE endpoint_t
{
	public:
		endpoint_t();

		endpoint_t(
			//! Имя конечной точки
			const std::string & endpoint_name );

		//! Получить имя конечной точки.
		inline const std::string &
		name() const
		{
			return m_endpoint_name;
		}

		//! Операторы сравнения.
		//! \{
		inline bool
		operator < ( const endpoint_t & endpoint ) const
		{
			return m_endpoint_name < endpoint.m_endpoint_name;
		}

		inline bool
		operator == ( const endpoint_t & endpoint ) const
		{
			return m_endpoint_name == endpoint.m_endpoint_name;
		}
		//! \}

		//! Изменить имя конечной точки.
		void
		set_name(
			//! Имя конечной точки
			const std::string & endpoint_name );

	private:
		std::string m_endpoint_name;
};

//! Тип для списка endpoint.
typedef std::vector< endpoint_t > endpoint_list_t;

//! Список стадий конечной точки.
typedef std::vector< std::string > stage_list_t;

//
// endpoint_stage_chain_t
//

//! Цепь Конечная точка.
/*!
	Сообщения в mbapi отправляются от одной конечной точки до другой.
	У конечной точки может быть несколько стадий.
	Отправляемые сообщения должны проходить все сдалии прежде чем
	попасть подписчику конечной точки - получателя сообщения.
	Если сообщение отправлено на заданную конечную точку, то оно сначала
	проходит через все стадии этой конечной точки в прямом порядке.
	Если сообщение отправлено с заданной конечной точки, то оно сначала
	проходит через стадии этой конечной точки в обратном порядке.

	Имена стадий не должны совпадать с именем конечной точки, а также
	не должны повторяться. В противном случае будет выбрасываться исключение.
*/
class MBAPI_4_DEFS_TYPE endpoint_stage_chain_t
{
	public:
		endpoint_stage_chain_t();

		endpoint_stage_chain_t(
			//! Конечная точка.
			const endpoint_t & endpoint );

		endpoint_stage_chain_t(
			//! Конечная точка.
			const endpoint_t & endpoint,
			//! Список стадий.
			const stage_list_t & stages );

		~endpoint_stage_chain_t();

		//! Создать endpoint_stage_chain_t из строки.
		/*!
			Данные о конесчной точке и стадиях
			должны иметь следующий формат:
			endpoint[stage1, stage2, ..., stageN].

			Например:
				\li ep[s1, s2, s3] - конечная точка - "ep",
				ее стадии "s1", "s2", "s3";
				\li ep - конечная точка - "ep", стадий не имеет.
				\li ep[] - конечная точка - "ep", стадий не имеет.
		*/
		static endpoint_stage_chain_t
		create_from_string(
			const std::string & s );

		//! Получить конечную точку.
		inline const endpoint_t &
		endpoint() const
		{
			return m_endpoint;
		}

		//! Получить имя конечной точки.
		inline void
		set_endpoint(
			const endpoint_t & endpoint )
		{
			m_endpoint = endpoint;
		}

		//! Получить список стадий.
		//! \{
		inline const stage_list_t &
		stages() const
		{
			return m_stages;
		}
		//! \}

		//! Установить список стадий.
		void
		set_stages(
			const stage_list_t & stages );

	private:
		//! Проверить не совпадает ли имя конечной точки
		//! с именем одной из стадий, а также уникальность
		//! имен самих стадий между собой.
		/*!
			В случае обнаружения совпадения выбрасывается исключение.
		*/
		void
		validate_stage_names() const;

		//! Имя конечной точки.
		endpoint_t m_endpoint;

		//! Список стадий заявленных конечной точкой.
		stage_list_t m_stages;
};

} /* namespace mbapi_4 */

#endif
