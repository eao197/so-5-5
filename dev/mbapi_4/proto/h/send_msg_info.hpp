/*
	MBAPI 4.
*/

/*!
	\file
	\brief Описания mbapi-пакета для отправки сообщений.
*/

#if !defined( _MBAPI_4__PROTO__SEND_MSG_INFO_HPP_ )
#define _MBAPI_4__PROTO__SEND_MSG_INFO_HPP_

#include <mbapi_4/comm/h/transmit_info.hpp>

#include <mbapi_4/proto/h/req_info.hpp>

namespace mbapi_4
{

namespace proto
{

//
// send_msg_info_t
//

/*!
	\brief Класс запроса на отправку сообщения.
*/
class send_msg_info_t
	:
		public req_info_t
{
		OESS_SERIALIZER( send_msg_info_t )

	public:
		//! Конструктор по умолчанию.
		send_msg_info_t();

		//! Инициализирующий конструктор.
		send_msg_info_t(
			comm::transmit_info_t & transmit_info );

		virtual ~send_msg_info_t();

		//! Имя конечной точки отправителя.
		std::string m_from;

		//! Имя конечной точки получателя.
		std::string m_to;

		//! Имя стадии.
		std::string m_current_stage;

		//! Флаг указывающий принадлежность стадии
		//! конечной точке m_from или m_to.
		/*!
			Если m_stagpoint_owner имеет значение 0, то
			владелец стадии m_from, иначе m_to
		*/
		oess_2::uchar_t m_stagpoint_owner;

		//! Выставить флаг приндлежности стадии.
		//! \{
		inline void
		set_stage_owner_is_from()
		{
			m_stagpoint_owner = 0;
		}

		inline void
		set_stage_owner_is_to()
		{
			m_stagpoint_owner = 1;
		}
		//! \}

		//! Получить имя конечной точки которой владеет данная стадия.
		inline const std::string &
		stage_owner_endpoint() const
		{
			if( 0 == m_stagpoint_owner )
				return m_from;

			return m_to;
		}

		//! Идентификатор типа сообщения.
		std::string m_oess_id;

		//! Бинарный образ сообщения.
		std::string m_payload;
};

//
// send_msg_resp_info_t
//

/*!
	\brief Класс ответа на отправку сообщения.
*/
class send_msg_resp_info_t
	:
		public resp_info_t
{
		OESS_SERIALIZER( send_msg_resp_info_t )

	public:
		//! Конструктор по умолчанию.
		send_msg_resp_info_t();

		//! Инициализирующий конструктор.
		/*! Для случая отсутствия сообщения об ошибке. */
		send_msg_resp_info_t(
			//! Код возврата отправки сообщения.
			int ret_code );

		//! Инициализирующий конструктор.
		send_msg_resp_info_t(
			//! Код возврата отправки сообщения.
			int ret_code,
			//! Текстовое описание ошибки.
			const std::string & error_msg );

		virtual ~send_msg_resp_info_t();
};

} /* namespace proto */

} /* namespace mbapi_4 */

#endif
