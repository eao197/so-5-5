/*
	MBAPI 4.
*/

/*!
	\file
	\brief Шаблон сообщения для передачи по mbapi.
*/

#if !defined( _MBAPI_4__MESSAGE_HPP_ )
#define _MBAPI_4__MESSAGE_HPP_

#include <memory>

#include <oess_2/stdsn/h/serializable.hpp>

#include <so_5/rt/h/message.hpp>

#include <mbapi_4/h/declspec.hpp>

#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/defs/h/oess_id_wrapper.hpp>

namespace mbapi_4
{

class mbapi_layer_t;

//
// message_base_t
//

//! Базовый класс mbapi сообщения.
/*!
	Служит для решения проблемы зависимости от определения
	mbapi_layer_t, которое в свою очередь, требует определения
	message_t. Поэтому mbapi_layer_t-зависимый код вынесен в
	файл имеплементации. Применить аналогичный прием напрямую к
	message_t, не получиться, потому что message_t является шаблонным классом.

*/
class MBAPI_4_TYPE message_base_t
	:
		public so_5::rt::message_t
{
	public:
		message_base_t(
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t & current_stage );

		virtual ~message_base_t();

		//! Информация о конечных точках между которыми идет сообщение.
		//! \{
		const endpoint_t &
		from() const
		{
			return m_from;
		}

		const endpoint_t &
		to() const
		{
			return m_to;
		}
		//! \}

		//! Имя текущей стадии.
		const stagepoint_t &
		current_stage() const
		{
			return m_current_stage;
		}

	protected:
		//! Реализация отправки.
		void
		move_next_impl(
			const oess_id_wrapper_t & oess_id,
			std::unique_ptr< oess_2::stdsn::serializable_t > msg ) const;

	private:
		//! Слой mbapi.
		mbapi_layer_t & m_mbapi_layer;

		//! Конечная точка с которой идет сообщение.
		const endpoint_t m_from;
		//! Конечная точка на которое идет сообщение.
		const endpoint_t m_to;

		//! Текущая стадия.
		const stagepoint_t m_current_stage;

};

//
// message_t
//

//! Класс обертка для передачи сообщений по mbapi.
template < class SERIALIZABLE >
class message_t
	:
		public message_base_t
{
		typedef message_base_t base_type_t;

	public:
		message_t(
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t & current_stage )
			:
				base_type_t( mbapi_layer, from, to, current_stage ),
				m_msg( new SERIALIZABLE )
		{}

		message_t(
			std::unique_ptr< SERIALIZABLE > msg,
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t & current_stage )
			:
				message_base_t( mbapi_layer, from, to, current_stage ),
				m_msg( std::move( msg ) )
		{}

		virtual ~message_t()
		{}

		//! Получить доступ к сообщению.
		//! \{
		SERIALIZABLE &
		msg()
		{
			return *m_msg;
		}

		inline const SERIALIZABLE &
		msg() const
		{
			return *m_msg;
		}
		//! \}

		//! Отправить сообщение дальше по цепочке.
		//! После отправки сообщения пользоваться им нельзя
		//! \{

		//! Отправить сообщение без изменений.
		//! Отправить измененное сообщение.
		void
		move_next(
			std::unique_ptr< SERIALIZABLE > msg ) const
		{
			if( msg.get() && !current_stage().is_endpoint() )
			{
				std::unique_ptr< oess_2::stdsn::serializable_t >
					message(
						static_cast< oess_2::stdsn::serializable_t * >(
							msg.release() ) );

				oess_id_wrapper_t oess_id(
					&SERIALIZABLE::oess_serializer_t::type_name() );

				move_next_impl(
					oess_id,
					std::move( message ) );
			}
		}

		void
		move_next() const
		{
			move_next( std::move( m_msg ) );
		}
		//! \}

	private:
		//! Сообщение.
		mutable std::unique_ptr< SERIALIZABLE > m_msg;
};


//
// binary_message_t
//

//! Класс бинарного сообщения, в котором
//! объект упакован с помощью oess.
/*!
	Служит для случаев, когда необходимо получать все сообщения
	независимо от типа.
	Подписка допускается только в эксклюзивном режиме,
	т.е. агент не должен быть подписан на кокретные типы
	mbapi_4 сообщений через данный endpoint или stagepoint.
*/
class MBAPI_4_TYPE binary_message_t
	:
		public message_base_t
{
		typedef message_base_t base_type_t;

	public:
		binary_message_t(
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t & current_stage,
			//! Идентификатор oess типа.
			const oess_id_wrapper_t & oess_id_wrapper,
			//! Упакованное с помощью oess сообщение.
			const std::string & bin_message );

		virtual ~binary_message_t();

		//! Идентификатор oess типа.
		oess_id_wrapper_t m_oess_id_wrapper;

		//! Упакованное с помощью oess сообщение.
		std::string m_bin_message;
};

} /* namespace mbapi_4 */

#endif
