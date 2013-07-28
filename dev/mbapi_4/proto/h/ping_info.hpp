/*
	MBAPI 4.
*/

/*!
	\file
	\brief Описания mbapi-пакета для переодических ping-ов.
*/

#if !defined( _MBAPI_4__PROTO__PING_INFO_HPP_ )
#define _MBAPI_4__PROTO__PING_INFO_HPP_

#include <mbapi_4/proto/h/req_info.hpp>

namespace mbapi_4
{

namespace proto
{

//
// ping_info_t
//

/*!
	\brief Класс запроса переодического ping-а.
*/
class ping_info_t
	:
		public req_info_t
{
		OESS_SERIALIZER( ping_info_t )

	public:
		//! Конструктор по умолчанию.
		ping_info_t();
		virtual ~ping_info_t();
};

//
// ping_resp_info_t
//

/*!
	\brief Класс ответа на переодический ping.
*/
class ping_resp_info_t
	:
		public resp_info_t
{
		OESS_SERIALIZER( ping_resp_info_t )

	public:
		//! Конструктор по умолчанию.
		ping_resp_info_t();
		virtual ~ping_resp_info_t();
};

} /* namespace proto */

} /* namespace mbapi_4 */

#endif
