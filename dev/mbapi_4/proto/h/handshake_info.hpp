/*
	MBAPI 4.
*/

/*!
	\file
	\brief Описания mbapi-пакета для процедуры handshake.
*/

#if !defined( _MBAPI_4__PROTO__HANDSHAKE_INFO_HPP_ )
#define _MBAPI_4__PROTO__HANDSHAKE_INFO_HPP_

#include <set>
#include <string>

#include <mbapi_4/proto/h/req_info.hpp>

namespace mbapi_4
{

namespace proto
{

namespace handshake
{

//
// version_t
//

//! Описание версии MBAPI.
struct version_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( version_t )

	public:
		//! Номер поколения.
		oess_2::uchar_t m_generation;
		//! Номер ветви.
		oess_2::uchar_t m_branch;
		//! Номер релиза.
		oess_2::uchar_t m_release;
		//! Номер публикации релиза.
		oess_2::uchar_t m_publish;

		//! Конструктор по-умолчанию.
		version_t();
		//! Полностью инициализирующий конструктор.
		version_t(
			oess_2::uchar_t generation,
			oess_2::uchar_t branch,
			oess_2::uchar_t release,
			oess_2::uchar_t publish );
		virtual ~version_t();

		//! Получение текущей версии SObjectizer-а.
		static version_t
		current();
};

//
// compression_t
//

//! Описание поддерживаемых алгоритмов компрессии данных.
struct compression_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( compression_t )

	public:
		//! Поддерживаемые клиентом алгоритмы компрессии.
		/*! Должен содержать хотя бы один элемент -- тот, который
			указан в m_preferred. */
		std::set< std::string >	m_supported;

		//! Предпочитаемый клиентом алгоритм компрессии.
		/*! Содержит пустое значение, если компрессия использоваться
			не должна. */
		std::string m_preferred;

	//! \name Поддерживаемые MBAPI алгоритмы компрессии.
	//! \{
		//! Алгоритм zlib.
		/*! См. http://www.gzip.org */
		static const std::string
		zlib();
	//! \}

		//! Конструктор по-умолчанию.
		/*!
			Устанавливает m_supported и m_preferred в пустые значения --
			компрессия не используется.
		*/
		compression_t();

		//! Инициализирующий конструктор.
		/*! В m_supported помещаются все элементы из \a supported
			и автоматически помещается значение \a preferred. */
		compression_t(
			//! Имя предпочитаемого алгоритма компрессии.
			const std::string & preferred,
			//! Другие поддерживаемые алгоритмы компрессии.
			const std::set< std::string > & supported );

		virtual ~compression_t();

		//! Проверка того, что компрессия используется.
		/*! \return true, если задано значение m_preferred и это
			значение находится в m_supported. */
		operator bool() const;

		//! Проверка того, что указанный алгоритм входит в число
		//! поддерживаемых алгоритмов.
		/*!
			\return true, если алгоритм перечислен в m_supported.
		*/
		bool
		is_supported( const std::string & name ) const;
};

} /* namespace handshake */

//
// handshake_info_t
//

/*!
	\brief Запрос операции handshake.

	В поле m_version передается версия MBAPI-а на стороне
	клиента (клиент инициирует операцию handshake).
*/
class handshake_info_t
	:
		public req_info_t
{
		OESS_SERIALIZER( handshake_info_t )

	public:
		//! Конструктор по умолчанию.
		/*! В качестве номера версии используется значение
			mbapi_4::proto::handshake::version_t::current(). */
		handshake_info_t();

		//! Инициализирующий конструктор.
		handshake_info_t(
			//! Поддерживаемая версия.
			const handshake::version_t & version );

		virtual ~handshake_info_t();

		//! Получить номер поддерживаемой версии.
		const handshake::version_t
		version() const;

		//! Получить описание желаемого режима компрессии.
		const handshake::compression_t
		compression() const;

		//! Установить желаемый режим компрессии.
		void
		set_compression(
			const handshake::compression_t & c );

		//! Получить идентификатор узла.
		const std::string &
		node_uid() const;

		//! Установить идентификатор узла.
		void
		set_node_uid( const std::string & uid );

	private:
		//! Номер версии MBAPI на стороне клиента.
		handshake::version_t	m_version;

		//! Желаемый режим компрессии.
		handshake::compression_t	m_compression;

		//! Идентификатор узла с которым установлен канал.
		std::string m_node_uid;
};

//
// handshake_resp_info_t
//

/*!
	\brief Результат операции handshake.

	В поле m_version передается версия MBAPI-а на стороне
	сервера (сервер обрабатывает операцию handshake).
*/
class handshake_resp_info_t
	:
		public resp_info_t
{
		OESS_SERIALIZER( handshake_resp_info_t )

	public:
		//! Конструктор по умолчанию.
		/*! В качестве номера версии используется значение
			mbapi_4::proto::handshake::version_t::current(). */
		handshake_resp_info_t();
		//! Инициализирующий конструктор.
		handshake_resp_info_t(
			//! Версия MBAPI на стороне сервера.
			const handshake::version_t & version );
		virtual ~handshake_resp_info_t();

		//! Получить номер версии сервера.
		const handshake::version_t
		version() const;

		//! Должна ли использоваться компрессия?
		/*! \return true, если режим компрессии задан и можно
			получить его название методом compression_algorithm(). */
		bool
		is_compression_defined() const;

		//! Выбранный сервером алгоритм компрессии.
		/*!
			\note возвращает корректное значение только, если
				true == is_compression_defined().
		*/
		const std::string &
		compression_algorithm() const;

		//! Установить название выбранного сервером алгоритма компрессии.
		/*!
			Пустое значение указывает, что компрессия не используется.
		*/

		void
		set_compression_algorithm(
			const std::string & name );

		//! Получить идентификатор узла.
		const std::string &
		node_uid() const;

		//! Установить идентификатор узла.
		void
		set_node_uid( const std::string & uid );

	private:
		//! Номер версии MBAPI на стороне сервера.
		handshake::version_t m_version;

		//! Имя выбранного сервером алгоритма компрессии.
		/*!
			Пустое значение указывает, что компрессия не используется.
		*/
		std::string m_compression_algorithm;

		//! Идентификатор узла с которым установлен канал.
		std::string m_node_uid;
};

} /* namespace proto */

} /* namespace mbapi_4 */

#endif
