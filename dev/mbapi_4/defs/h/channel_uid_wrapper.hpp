/*
	MBAPI 4.
*/

/*!
	\file
	\brief ќбертка дл€ уникального идентификатора канала.
*/

#if !defined( _MBAPI_4__DEFS__CHANNEL_UID_WRAPPER_HPP_ )
#define _MBAPI_4__DEFS__CHANNEL_UID_WRAPPER_HPP_

#include <string>

#include <mbapi_4/defs/h/declspec.hpp>

namespace mbapi_4
{

//
// channel_uid_wrapper_t
//

//! ќбертка над uid-ом канала.
/*!
	Mbapi кнал представл€ет собой св€зь установленную
	между двум€ mbapi-узлами. ≈сли два узла имеют междусобой
	более чем одно соединение, то каждое соединение считаетс€ отдельным каналом.
	Ќапример если узел A открыл серверный канал, а узел B
	установил с ним два клиентских соединени€, то каждое соединение
	будет отдельными каналами, как со стороны A, так и со стороны B.
	»дентификатор канала имеет значение только внутри данного узла,
	он никогда не передаетс€, и уникальность необходима только
	в рамках конкретного mbapi-узла.

	ƒл€ представлени€ локальных точек данного узла,
	существует пон€тие "локальный" канал, который значит, что
	точка располагаетс€ на данном узле и дл€ передачи
	сообщени€ таким точкам не нужно передавать собщени€ по сети.
*/
class MBAPI_4_DEFS_TYPE channel_uid_wrapper_t
{
	public:
		channel_uid_wrapper_t();

		explicit channel_uid_wrapper_t(
			const std::string & uid );

		~channel_uid_wrapper_t();

		inline bool
		operator < ( const channel_uid_wrapper_t & channel_uid ) const
		{
			return m_uid < channel_uid.m_uid;
		}

		inline bool
		operator == ( const channel_uid_wrapper_t & channel_uid ) const
		{
			return m_uid == channel_uid.m_uid;
		}

		inline bool
		operator != ( const channel_uid_wrapper_t & channel_uid ) const
		{
			return m_uid != channel_uid.m_uid;
		}

		//! явл€етс€ ли канал локальным.
		inline bool
		is_local() const
		{
			return m_uid.empty();
		}

		//! —“роковое представление идентификатора узла.
		inline const std::string &
		str() const
		{
			return m_uid;
		}

	private:
		//! »дентификатор канала.
		std::string m_uid;
};

} /* namespace mbapi_4 */

#endif
