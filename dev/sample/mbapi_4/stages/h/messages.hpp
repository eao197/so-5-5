/*
	MBAPI 4 Sample
*/

/*!
	Типы передаваемых сообщений.
*/

#if !defined( _SAMPLE__MBAPI_4__STAGES__MESSAGES_HPP_ )
#define _SAMPLE__MBAPI_4__STAGES__MESSAGES_HPP_

#include <string>

#include <oess_2/defs/h/types.hpp>
#include <oess_2/stdsn/h/serializable.hpp>

namespace sample_mbapi_4
{

struct ping_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( ping_t )

	public:
		//! Время когда отправлен ping.
		std::string m_timestamp;
		//! Номер.
		oess_2::uint_t m_num;

		//! Стадии через которые прошло сообщение.
		std::vector< std::string > m_stages_passed;
};

struct pong_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( pong_t )

	public:
		//! Время когда отправлен pong.
		std::string m_timestamp;

		//! Номер.
		oess_2::uint_t m_num;

		//! Стадии через которые прошло сообщение.
		std::vector< std::string > m_stages_passed;
};


} /* namespace sample_mbapi_4 */

#endif
