/*
	SObjectizer 5 SOP4
*/

/*!
	\file
	\brief Базовые классы и определения для поддержки binSOP.
*/

#if !defined( _MBAPI_4__PROTO__REQ_INFO_HPP_ )
#define _MBAPI_4__PROTO__REQ_INFO_HPP_

#include <memory>
#include <string>
#include <deque>

#include <oess_2/stdsn/h/serializable.hpp>
#include <oess_2/stdsn/h/shptr.hpp>

namespace mbapi_4
{

namespace proto
{

//
// req_info_t
//

/*!
	\brief Базовый класс для элементов mbapi-пакетов.

	От этого класса производятся как классы-запросы,
	так и классы-ответы. Классы-запросы производятся
	непосредственно от %req_info_t, а классы-ответы
	производятся от resp_info_t.
*/
class req_info_t
	:
		public oess_2::stdsn::serializable_t
{
	OESS_SERIALIZER( req_info_t )
	public:
		//! Конструктор по умолчанию.
		/*!
			Обнуляет идентификатор запроса.
		*/
		req_info_t();
		//! Полностью инициализирующий конструктор.
		req_info_t(
			//! Идентификатор запроса/ответа
			oess_2::int_t req_id );
		virtual ~req_info_t();

		//! Опредение идентификатора элемента SOP.
		/*!
			\return Элемент перечисления req_id_t.
		*/
		oess_2::int_t
		query_req_id() const;

	private:
		//! Идентификатор элемента SOP.
		/*! Элемент перечисления req_id_t. */
		oess_2::int_t	m_req_id;
};

//
// req_info_ptr_t
//

/*!
	\breif Разделяемый указатель на %req_info_t.
	Детализация oess_2::stdsn::shptr_t для req_info_t.
*/
class req_info_ptr_t
	:
		public oess_2::stdsn::shptr_t
{
	OESS_SERIALIZER( req_info_ptr_t )
	OESS_2_SHPTR_IFACE( req_info_ptr_t,
		mbapi_4::proto::req_info_t,
		oess_2::stdsn::shptr_t )
};

//
// resp_info_t
//

/*!
	\brief Базовый класс для классов-ответов.

	В ответе на запрос всегда передается код возврата и,
	возможно, строка с описанием ошибки. Хранение этих
	полей вынесено в базовый для всех ответов класс.
*/
class resp_info_t
	:
		public req_info_t
{
		OESS_SERIALIZER( resp_info_t )

	public:
		//! Конструктор по умолчанию.
		/*! Обнуляет req_id и ret_code */
		resp_info_t();

		//! Инициализирующий конструктор.
		/*! Для случая, когда нет описания ошибки. */
		resp_info_t(
			//! Идентификатор элемента.
			oess_2::int_t req_id,
			//! Код возврата.
			oess_2::int_t ret_code );

		//! Инициализирующий конструктор.
		resp_info_t(
			//! Идентификатор элемента.
			oess_2::int_t req_id,
			//! Код возврата.
			oess_2::int_t ret_code,
			//! Текстовое описание ошибки.
			const std::string & error_msg );

		virtual ~resp_info_t();

		//! Определить код возврата обработки запроса.
		oess_2::int_t
		query_ret_code() const;

		//! Определить описание ошибки.
		/*! Возвращает пустую строку, если описание
			ошибки не было передано с удаленного узла. */
		const std::string &
		query_error_msg() const;

	private:
		//! Код возврата обработки запроса.
		oess_2::int_t m_ret_code;
		//! Описание ошибки.
		std::string m_error_msg;
};

//
// unknown_resp_info_t
//

/*!
	\brief Класс ответа на неизвестный пакет.

	Если по SOP приходит неизвестный пакет, либо пакет не
	удается разобрать, то в ответ отсылается объект
	этого типа.
*/
class unknown_resp_info_t
	:
		public resp_info_t
{
		OESS_SERIALIZER( unknown_resp_info_t )

	public:
		//! Конструктор по умолчанию.
		unknown_resp_info_t();
		//! Полностью инициализирующий конструктор.
		unknown_resp_info_t(
			//! Код возврата.
			oess_2::int_t ret_code,
			//! Текстовое описание ошибки.
			const std::string & error_msg );
		virtual ~unknown_resp_info_t();
};

//
// req_info_storage_t
//

/*!
	\brief Хранилище элементов SOP.
*/
typedef std::deque< req_info_ptr_t > req_info_storage_t;

namespace req_id
{

//
// req_id_t
//

/*!
	\brief Идентификаторы элементов SOP.

	Формат: aarrvv, где:
	- aa - запрос (0x00) или ответ (0x01)
	- rr - код операции
	- vv - версия SOP (текущая - 0x07)
*/
enum req_id_t
{
	//! Запрос на отсылку сообщения.
	REQ_SEND_MSG = 0x000105,

	//! Ответ на попытку отсылки сообщения.
	RESP_SEND_MSG = 0x010105,

	//! Элементом является объект unknown_resp_info_t.
	RESP_UNKNOWN = 0x010005,

	//! Запрос ping.
	REQ_PING = 0x000406,

	//! Ответ на ping.
	RESP_PING = 0x010406,

	//! Начало процедуры handshake.
	REQ_HANDSHAKE = 0x000507,

	//! Ответ на процедуру handshake.
	RESP_HANDSHAKE = 0x010507,

	//! Запрос на синхронизацию маршрутных таблиц.
	REQ_SYNC_TABLES = 0x000607,

	//! Ответ на синхронизацию маршрутных таблиц.
	RESP_SYNC_TABLES = 0x010607
};

} /* namespace req_id */

} /* namespace proto */

} /* namespace mbapi_4 */

#endif
