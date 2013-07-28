/*
	MBAPI 4.
*/

/*!
	\file
	\brief Класс для работы с заголовками пакетов протокола MBAPI.
*/

#if !defined( _MBAPI_4__PROTO__PACKAGE_HEADER_HPP_ )
#define _MBAPI_4__PROTO__PACKAGE_HEADER_HPP_

#include <oess_2/io/h/binstream.hpp>

namespace mbapi_4
{

namespace proto
{

//
// package_header_t
//

/*!
	\brief Класс для обработки заголовка mbapi-пакета.

	Каждая порция данных в SOP обрамляется в виде пакета со
	специальным заголовком в начале. Данный заголовок содержит
	размер содержательной части пакета в байтах и специальное
	контрольное поле.

	Размер содержательной части используется для управления
	чтением пакета из коммуникационного канала. Т.е. сначала
	считывается заголовок пакета, который имеет заранее
	определенный размер. Затем считывается столько байт, сколько
	указано в заголовке пакета. После чего пакет считается
	полностью прочитанным.

	Контрольное поле используется для проверки того, что
	читается действительно mbapi пакет подходящей версии.
*/
class package_header_t
{
	public:
		//! 32-битовое беззнаковое целое.
		typedef unsigned int	uint32_t;

		enum
		{
			//! Размер двоичного представления размера
			//! пакета в байтах.
			IMAGE_SIZE = 8
		};

	public:
		//
		// magic_number
		//

		//! Хранитель значения для специфического magic_number.
		/*!
			Предназначен для разрешения неоднозначностей вызова
			конструктора с одним аргументом.
		*/
		struct magic_number_t
		{
			//! Специфическое значение для magic_number.
			uint32_t m_value;

			//! Инициализирующий конструктор.
			explicit inline
			magic_number_t( uint32_t value )
				:
					m_value( value )
			{}
		};

		//! Конструктор, который позволяет установить только значение
		//! magic_number.
		package_header_t( const magic_number_t & value );

		//! Конструктор, который позволяет установить значение
		//! magic_number и длину содержательной части пакета.
		package_header_t(
			//! Значение magic_number.
			const magic_number_t & value,
			//! Размер содержательной части пакета в байтах.
			uint32_t package_size );

		//! Проверка корректности заголовка пакета.
		/*!
			\return true, если заголовок является корректным.
		*/
		bool
		is_valid() const;

		//! Длина содержательной части пакета в байтах.
		uint32_t
		size() const;

		//! Установить длину содержательной части пакета.
		void
		set_size(
			//! Размер в байтах.
			uint32_t package_size );

		//! Прочитать заголовок пакета из указанного потока.
		void
		read(
			//! Чтение осуществляется с текущей позиции.
			oess_2::io::ibinstream_t & from );

		//! Записать заголовок пакета в указанный поток.
		void
		write(
			//! Запись осуществляется в текущую позицию.
			oess_2::io::obinstream_t & to ) const;

		/*!
		 * \brief Создание отладочного представления.
		 *
		 * Используется при печати проблемных заголовков.
		 */
		std::string
		debug_string_representation() const;

	private:
		//! Значение magic_number.
		uint32_t	m_magic_number;
		//! Размер содержательной части пакета в байтах.
		uint32_t	m_size;
		//! Контрольное поле.
		uint32_t	m_check_field;

		//! Пересчитать значение контрольного поля.
		void
		calc_check_field();
};

//! Более простой способ записи заголовка пакета в поток.
inline oess_2::io::obinstream_t &
operator << (
	oess_2::io::obinstream_t & o,
	const package_header_t & h )
{
	h.write( o );
	return o;
}

//! Более простой способ чтения заголовка пакета из потока.
inline oess_2::io::ibinstream_t &
operator >> (
	oess_2::io::ibinstream_t & o,
	package_header_t & h )
{
	h.read( o );
	return o;
}

} /* namespace proto */

} /* namespace mbapi_4 */

#endif
