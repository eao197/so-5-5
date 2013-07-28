/*
	MBAPI 4.
*/

/*!
	\file
	\brief Вспомогательные определения типов.
*/

#if !defined( _MBAPI_4__COMM__TRANSMIT_INFO_HPP_ )
#define _MBAPI_4__COMM__TRANSMIT_INFO_HPP_

#include <string>

#include <oess_2/defs/h/types.hpp>
#include <oess_2/stdsn/h/serializable.hpp>

#include <so_5/rt/h/message.hpp>

#include <mbapi_4/h/declspec.hpp>

#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/defs/h/endpoint.hpp>

#include <mbapi_4/defs/h/oess_id_wrapper.hpp>
#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>


namespace mbapi_4
{

namespace comm
{

//! Тип для хранения бинарного представления данных.
typedef std::string raw_data_t;

//
// transmit_info_t
//

//! Информация о передаваемом сообщении.
/*!
	Служит для обращиния в mbapi системе
	сообщений, которые поступили из каналов.
	В случае востребованности сообщения на данном узле
	оно будет вычитано из m_payload. Если
	же сообдение должно быть просто ретранслировано
	в другой канал, то его бинарный образ передается как есть.
*/
struct MBAPI_4_TYPE transmit_info_t
	:
		public so_5::rt::message_t
{
	transmit_info_t(
		//! Конечная точка с которой идет сообщение.
		const endpoint_t & from,
		//! Конечная точка на которое идет сообщение.
		const endpoint_t & to,
		//! Текущая стадия.
		const stagepoint_t & current_stage,
		//! Идентификатор типа.
		const oess_id_wrapper_t & oess_id )
		:
			m_from( from ),
			m_to( to ),
			m_current_stage( current_stage ),
			m_oess_id( oess_id )
	{}

	transmit_info_t(
		//! Конечная точка с которой идет сообщение.
		const std::string & from,
		//! Конечная точка на которое идет сообщение.
		const std::string & to,
		//! Текущая стадия.
		const std::string & current_stage_name,
		//! Текущая стадия.
		const std::string & current_stage_endpoint_name,
		//! Идентификатор типа.
		const std::string & oess_id )
		:
			m_from( from ),
			m_to( to ),
			m_current_stage( current_stage_name, current_stage_endpoint_name ),
			m_oess_id( oess_id )
	{}

	virtual ~transmit_info_t();

	//! Идентификатор канала в который надо отправить сообщение.
	/*!
		Значение имеет смысл, только в случае, если сообщение
		маршрутизируется mbapi_layer_impl_t в другой канал.
	*/
	channel_uid_wrapper_t m_channel_id;

	//! Конечная точка, с которой отправлено сообщение.
	endpoint_t m_from;
	//! Конечная точка, на которую отправлено сообщение.
	endpoint_t m_to;

	//! Текущая стадия.
	/*!
		При самом начале отправки сообщения
		m_current_stage - это m_from.
	*/
	stagepoint_t m_current_stage;

	//! OESS идентификатор сообщения.
	oess_id_wrapper_t m_oess_id;

	//! Бинарное представление сообщения.
	raw_data_t m_payload;
};

//! Псевдоним unique_ptr для transmit_info_t.
typedef std::unique_ptr< transmit_info_t > transmit_info_unique_ptr_t;

//! Информация о передаваемом сообщении.
/*!
	Служит для обращиния в mbapi системе
	сообщений, которые созданы и отправлены с конечной точки
	расположенной в данном узле, при необходимости
	отправки сообщения в канал сообщение трансформируется в transmit_info_t.
*/
struct MBAPI_4_TYPE local_transmit_info_t
	:
		public so_5::rt::message_t
{
	local_transmit_info_t(
		//! Конечная точка с которой идет сообщение.
		const endpoint_t & from,
		//! Конечная точка на которое идет сообщение.
		const endpoint_t & to,
		//! Текущая стадия.
		const stagepoint_t & current_stage,
		//! Идентификатор типа.
		const oess_id_wrapper_t & oess_id,
		//! СОобщение.
		std::unique_ptr< oess_2::stdsn::serializable_t > message )
		:
			m_from( from ),
			m_to( to ),
			m_current_stage( current_stage ),
			m_oess_id( oess_id ),
			m_message( std::move( message ) )
	{}

	virtual ~local_transmit_info_t();

	//! Конечная точка, с которой отправлено сообщение.
	endpoint_t m_from;
	//! Конечная точка, на которую отправлено сообщение.
	endpoint_t m_to;

	//! Текущая стадия.
	/*!
		При самом начале отправки сообщения
		m_current_stage - это m_from.
	*/
	stagepoint_t m_current_stage;

	//! OESS идентификатор сообщения.
	oess_id_wrapper_t m_oess_id;

	//! Указатель на объект сообщения.
	std::unique_ptr< oess_2::stdsn::serializable_t > m_message;

};

//! Псевдоним unique_ptr для transmit_info_t.
typedef std::unique_ptr< local_transmit_info_t > local_transmit_info_unique_ptr_t;

} /* namespace comm */

} /* namespace mbapi_4 */

#endif
