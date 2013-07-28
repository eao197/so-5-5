/*
	MBAPI 4.
*/

/*!
	\file
	\brief Сертификат точки-стадии на данном узле.
*/

#if !defined( _MBAPI_4__STAGEPOINT_BIND_HPP_ )
#define _MBAPI_4__STAGEPOINT_BIND_HPP_

#include <oess_2/stdsn/h/ent.hpp>

#include <so_5/rt/h/subscription_bind.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/mbox_ref.hpp>

#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/defs/h/oess_id_wrapper.hpp>

#include <mbapi_4/h/declspec.hpp>

#include <mbapi_4/h/message.hpp>

namespace mbapi_4
{

namespace impl
{

// Реализация сертификата подписки.
class stagepoint_bind_impl_t;

} /* namespace impl */

class mbapi_layer_t;

//! Вспомогательный класс-интерфейс для выполнения подписки.
class MBAPI_4_TYPE event_subscriber_t
{
		event_subscriber_t(
			const event_subscriber_t & );
		void
		operator = (
			const event_subscriber_t & );

	public:
		event_subscriber_t(
			const so_5::rt::state_t & state );
		virtual ~event_subscriber_t();

		//! Выполнить подписку обработчика данного агента на заданное сообщение.
		//! Обработчик, состояние агента и тип сообщения должны определять потомки.
		virtual so_5::ret_code_t
		subscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox ) = 0;

		//! Выполнить отмену подписки обработчика данного агента на заданное сообщение.
		//! Обработчик, состояние агента и тип сообщения должны определять потомки.
		virtual so_5::ret_code_t
		unsubscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox ) = 0;

		//! Состояние агента в котором необходимо осуществлять одписку.
		inline const so_5::rt::state_t &
		state() const
		{
			return m_state;
		}

	protected:
		const so_5::rt::state_t & m_state;
};

//! Реализация подписчика для конкретных типов сообщений.
template< class MBAPI_MESSAGE, class AGENT >
class real_event_subscriber_t
	:
		public event_subscriber_t
{
	public:
		//! Указатель на метод обработчик события агента.
		typedef void (AGENT::*FN_PTR_T)(
			const so_5::rt::event_data_t< MBAPI_MESSAGE > & );

		real_event_subscriber_t(
			FN_PTR_T pfn,
			const so_5::rt::state_t & state )
			:
				event_subscriber_t( state ),
				m_pfn( pfn )
		{}

		virtual ~real_event_subscriber_t()
		{}

		virtual so_5::ret_code_t
		subscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox )
		{
			so_5::rt::subscription_bind_t subscription_bind( agent, mbox );
			return subscription_bind
				.in( m_state )
					.event(
						m_pfn,
						so_5::DO_NOT_THROW_ON_ERROR );
		}

		virtual so_5::ret_code_t
		unsubscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox )
		{
			so_5::rt::subscription_unbind_t subscription_bind( agent, mbox );
			return subscription_bind
				.in( m_state )
					.event(
						m_pfn,
						so_5::DO_NOT_THROW_ON_ERROR );
		}
	private:
		FN_PTR_T m_pfn;
};

//! Вспомогательный класс-интерфейс для вычитывания и отправки сообщения.
class MBAPI_4_TYPE message_integrator_t
{
	public:
		virtual ~message_integrator_t();

		//! Вычитать сообщение и отправить
		virtual void
		read_and_send(
			//! Слой mbapi.
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t & current_stage,
			//! Читатель объекта.
			oess_2::stdsn::ient_t & reader,
			//! Mbox, на который следует отправить сообщение.
			so_5::rt::mbox_ref_t & mbox ) const = 0;

		//! Преобразовать сообщение и отпраить.
		virtual void
		cast_and_send(
			//! Слой mbapi.
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t & to,
			//! Текущая стадия.
			const stagepoint_t &  current_stage,
			//! Объект для сообщения.
			std::unique_ptr< oess_2::stdsn::serializable_t > msg,
			//! Mbox, на который следует отправить сообщение.
			so_5::rt::mbox_ref_t & mbox ) const = 0;
};

//! Реализация интегратора сообщений для конкретных типов.
template < class SERIALIZABLE >
class message_integrator_impl_t
	:
		public message_integrator_t
{
		typedef mbapi_4::message_t< SERIALIZABLE > MBAPI_MESSAGE;

	public:
		virtual ~message_integrator_impl_t()
		{}

		virtual void
		read_and_send(
			//! Слой mbapi.
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t &  from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t &  to,
			//! Текущая стадия.
			const stagepoint_t & current_stage,
			//! читатель объекта.
			oess_2::stdsn::ient_t & reader,
			//! Mbox, на который следует отправить сообщение.
			so_5::rt::mbox_ref_t & mbox ) const
		{
			std::unique_ptr< MBAPI_MESSAGE >
				mbapi_message(
					new MBAPI_MESSAGE(
						mbapi_layer,
						from,
						to,
						current_stage ) );

			reader >> mbapi_message->msg();

			mbox->deliver_message( mbapi_message );
		}

		virtual void
		cast_and_send(
			//! Слой mbapi.
			mbapi_layer_t & mbapi_layer,
			//! Конечная точка с которой идет сообщение.
			const endpoint_t & from,
			//! Конечная точка на которое идет сообщение.
			const endpoint_t &  to,
			//! Текущая стадия.
			const stagepoint_t & current_stage,
			//! Объект сообщения.
			std::unique_ptr< oess_2::stdsn::serializable_t > msg,
			//! Mbox, на который следует отправить сообщение.
			so_5::rt::mbox_ref_t & mbox ) const
		{
			std::unique_ptr< SERIALIZABLE >
				casted_message(
					dynamic_cast< SERIALIZABLE * >( msg.get() ) );

			if( 0 != casted_message.get() )
			{
				msg.release();

				std::unique_ptr< MBAPI_MESSAGE > mbapi_message(
					new MBAPI_MESSAGE(
						std::move( casted_message ),
						mbapi_layer,
						from,
						to,
						current_stage ) );

				mbox->deliver_message(
					std::move( mbapi_message ) );
			}
		}
};

//
// stagepoint_bind_t
//

//! Привязка к точке-стадии.
/*!
	Представляет собой образ стадии на данном узле какой-либо конечной точки.
	С помощью привязки можно подписываться на сообщения
	адресуемые данной конечной точке или отправленные из данной конечной точки.

	Привязки выдаются mbapi_layer_t, который контролирует их уникальность.
	Для привязки необходимо укзать агента, и подписаться на
	сообщения, в последствии, можно только обработчиками данного агента.

	При уничтожении привязки точка-стадия
	автоматически перестает существовать на данном узле.

	С помощью привязки точки-стадии можно только подписываться
	на сообщения, но не отправлять их. Отправлять же можно только
	полученые сообщения и только далее по цепочке стадий.
*/
class MBAPI_4_TYPE stagepoint_bind_t
{
	friend class mbapi_layer_t;
	friend class endpoint_bind_t;

	stagepoint_bind_t( const stagepoint_bind_t & );
	void
	operator = ( const stagepoint_bind_t & );

	//! Защищенный конструктор, потому что созданием привязки
	//! может заниматься только слой mbapi.
	stagepoint_bind_t(
		std::unique_ptr< impl::stagepoint_bind_impl_t > impl );

	public:
		virtual ~stagepoint_bind_t();

		//! Получить наименование точки-стадии.
		const stagepoint_t &
		stagepoint() const;

		// TODO: Можно ли сделать так чтобы при подписке
		// было не обязательно указывать состояние агента,
		// чтобы в этом случае бралось состояние по умолчанию.
		//! Подписаться на получение сообщения.
		template< class AGENT, class SERIALIZABLE >
		so_5::ret_code_t
		subscribe_event(
			//! Состояние агента.
			const so_5::rt::state_t & state,
			//! Метод обработки сообщения.
			void (AGENT::*pfn)(
				const so_5::rt::event_data_t< message_t< SERIALIZABLE >  > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy =
				so_5::THROW_ON_ERROR )
		{
			real_event_subscriber_t< message_t< SERIALIZABLE >, AGENT >
				real_event_subscriber(
					pfn,
					state );

			const oess_id_wrapper_t oess_id(
				&SERIALIZABLE::oess_serializer_t::type_name() );

			return subscribe_event_impl(
				oess_id,
				real_event_subscriber,
				std::unique_ptr< message_integrator_t >(
					new message_integrator_impl_t< SERIALIZABLE > ),
				throwing_strategy );
		}

		//! Подписаться на получение бинарных сообщений.
		template< class AGENT >
		so_5::ret_code_t
		subscribe_event(
			//! Состояние агента.
			const so_5::rt::state_t & state,
			//! Метод обработки сообщения.
			void (AGENT::*pfn)(
				const so_5::rt::event_data_t< binary_message_t > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy =
				so_5::THROW_ON_ERROR )
		{
			real_event_subscriber_t< binary_message_t, AGENT >
				real_event_subscriber(
					pfn,
					state );

			return bin_msg_subscribe_event_impl(
				real_event_subscriber,
				throwing_strategy );
		}

		//! Отменить подписку на получение сообщения.
		template< class AGENT, class SERIALIZABLE >
		so_5::ret_code_t
		unsubscribe_event(
			//! Состояние агента.
			const so_5::rt::state_t & state,
			//! Метод обработки сообщения.
			void (AGENT::*pfn)(
				const so_5::rt::event_data_t< message_t< SERIALIZABLE >  > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy =
				so_5::THROW_ON_ERROR )
		{
			real_event_subscriber_t< message_t< SERIALIZABLE >, AGENT >
				real_event_subscriber(
					pfn,
					state );

			const oess_id_wrapper_t oess_id(
				&SERIALIZABLE::oess_serializer_t::type_name() );

			return unsubscribe_event_impl(
				oess_id,
				real_event_subscriber,
				std::unique_ptr< message_integrator_t >(
					new message_integrator_impl_t< SERIALIZABLE > ),
				throwing_strategy );
		}

		//! Отменить подписку на получение бинарных сообщений.
		template< class AGENT >
		so_5::ret_code_t
		unsubscribe_event(
			//! Состояние агента.
			const so_5::rt::state_t & state,
			//! Метод обработки сообщения.
			void (AGENT::*pfn)(
				const so_5::rt::event_data_t< binary_message_t > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy =
				so_5::THROW_ON_ERROR )
		{
			real_event_subscriber_t< binary_message_t, AGENT >
				real_event_subscriber(
					pfn,
					state );

			return bin_msg_unsubscribe_event_impl(
				real_event_subscriber,
				throwing_strategy );
		}

	private:
		//! Выполнить подписку.
		so_5::ret_code_t
		subscribe_event_impl(
			//! Тип сообщения.
			const oess_id_wrapper_t & oess_id,
			//! Реализатор подписки агента.
			event_subscriber_t & event_subscriber,
			//! Интегратор сообщения.
			std::unique_ptr< message_integrator_t > message_integrator,
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy );

		//! Выполнить отмену подписки.
		so_5::ret_code_t
		unsubscribe_event_impl(
			//! Тип сообщения.
			const oess_id_wrapper_t & oess_id,
			//! Реализатор подписки агента.
			event_subscriber_t & event_subscriber,
			//! Интегратор сообщения.
			std::unique_ptr< message_integrator_t > message_integrator,
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy );

		//! Выполнить подписку.
		so_5::ret_code_t
		bin_msg_subscribe_event_impl(
			//! Реализатор подписки агента.
			event_subscriber_t & event_subscriber,
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy );

		//! Выполнить отмену подписки.
		so_5::ret_code_t
		bin_msg_unsubscribe_event_impl(
			//! Реализатор подписки агента.
			event_subscriber_t & event_subscriber,
			//! Флаг - бросать ли исключение в случае ошибки.
			so_5::throwing_strategy_t throwing_strategy );

	protected:
		//! Внутренняя реализация.
		std::unique_ptr< impl::stagepoint_bind_impl_t >
			m_impl;
};

//! Псевдоним unique_ptr для stagepoint_bind_t
typedef std::unique_ptr< stagepoint_bind_t > stagepoint_bind_unique_ptr_t;

} /* namespace mbapi_4 */

#endif
