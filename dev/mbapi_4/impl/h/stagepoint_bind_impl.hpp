/*
	MBAPI 4.
*/

/*!
	\file
	\brief Реадизация привязки точки-стадии на данном узле.
*/

#if !defined( _MBAPI_4__IMPL__STAGEPOINT_BIND_IMPL_HPP_ )
#define _MBAPI_4__IMPL__STAGEPOINT_BIND_IMPL_HPP_

#include <vector>
#include <map>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/mbox_ref.hpp>

#include <mbapi_4/defs/h/endpoint.hpp>

#include <mbapi_4/comm/h/transmit_info.hpp>

#include <mbapi_4/h/message.hpp>
#include <mbapi_4/h/stagepoint_bind.hpp>

namespace mbapi_4
{

namespace impl
{

// Реализация слоя работы с mbapi_4.
class mbapi_layer_impl_t;

//! Тип для списка состояний.
typedef std::vector< const so_5::rt::state_t * > state_list_t;

//! Тип умного указателя для state_list_t.
typedef std::shared_ptr< state_list_t >
	state_list_ptr_t;

//! Тип умного указателя для message_integrator_t.
typedef std::shared_ptr< message_integrator_t >
	message_integrator_ptr_t;

//! Информация о подписке агента на заданный тип сообщений.
struct subscription_info_t
{
	subscription_info_t()
		:
			m_states(  new state_list_t )
	{}

	inline bool
	has_state(
		const so_5::rt::state_t & state ) const
	{
		return m_states->end() !=
			std::find( m_states->begin(), m_states->end(), &state );
	}

	void
	add_state( const so_5::rt::state_t & state )
	{
		m_states->push_back( &state );
	}

	bool
	remove_state( const so_5::rt::state_t & state )
	{
		state_list_t::iterator it =
			std::find( m_states->begin(), m_states->end(), &state );

		if( m_states->end() != it )
		{
			m_states->erase( it );
			return true;
		}

		return false;
	}

	bool
	empty() const
	{
		return m_states->empty();
	}

	//! Список состояний, в которых агент подписан на заданный тип сообщений.
	state_list_ptr_t m_states;

	//! Интегратор сообщений.
	message_integrator_ptr_t m_message_integrator;
};

//! Тип для таблицы типов сообщения и
//! дополнительной информации по подписке агента на данный тип сообщений.
struct subscribed_types_table_t
{
	//! Есть ли подписки на типизированные сообщения.
	inline bool
	has_subscribed_types() const
	{
		return !m_subscribed_types_map.empty();
	}

	//! Если ли подписка на бинарное сообщение.
	inline bool
	binary_message_subscribed() const
	{
		return !m_binary_message_subscription_info.empty();
	}

	typedef std::map< oess_id_wrapper_t, subscription_info_t >
		subscribed_types_map_t;
	subscribed_types_map_t m_subscribed_types_map;

	subscription_info_t m_binary_message_subscription_info;
};

//
// stagepoint_bind_impl_t
//

//! Привязка стадии.
/*!
	Реализация привязки для использования
	mbapi_4::stagepoint_bind_t и mbapi_4::endpoint_bind_t.
*/
class stagepoint_bind_impl_t
{
	friend class mbapi_layer_impl_t;

	stagepoint_bind_impl_t( const stagepoint_bind_impl_t & );
	void
	operator = ( const stagepoint_bind_impl_t & );

	//! Созданием привязки может заниматься только слой mbapi.
	stagepoint_bind_impl_t(
		//! Наименование точки.
		const stagepoint_t & stagepoint,
		//! Агент, которому выдается привязка.
		so_5::rt::agent_t & agent,
		//! Слой mbapi, который создал данную привязку.
		impl::mbapi_layer_impl_t & mbapi_layer );

	public:
		virtual ~stagepoint_bind_impl_t();

		//! Получить наименование конечной точки.
		inline const stagepoint_t &
		stagepoint() const
		{
			return m_stagepoint;
		}

		//! Выполнить подписку на сообщение.
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

		//! Выполнить отправку сообщения.
		/*!
			Используется только для обслуживания endpoint-ов.
		*/
		void
		send_impl(
			const endpoint_t & to,
			const oess_id_wrapper_t & oess_id,
			std::unique_ptr< oess_2::stdsn::serializable_t > msg );

		//! Выполнить отправку бинарного сообщения.
		/*!
			Используется только для обслуживания endpoint-ов.
		*/
		void
		send_binary_message_impl(
			const endpoint_t & to,
			const oess_id_wrapper_t & oess_id,
			const std::string & binary_message );

		//! Попытаться принять сообщение.
		/*!
			Если точка подписана на заданный тип сообщений,
			то его содержимое вычитывается и происходит отправка.

			Если же на тип сообщения нет никакой подписки,
			то оно игнорируется. В этом случае его надо
			передавать далее по цепочке.

			\return Если сообщение принято, то возвращается true,
			в противном случае false.
		*/
		bool
		try_to_accept_message(
			oess_2::stdsn::ient_t & reader,
			const comm::transmit_info_t & transmit_info ) const;

		//! Попытаться принять сообщение.
		/*!
			Если точка подписана на заданный тип сообщений,
			то делается приведение типов и производиться
			отправка сообщения.

			Если же на тип сообщения нет никакой подписки,
			то оно игнорируется. В этом случае его надо
			передавать далее по цепочке.

			\return Если сообщение принято, то возвращается true,
			в противном случае false.
		*/
		bool
		try_to_accept_message(
			comm::local_transmit_info_t & local_transmit_info ) const;

	private:
		so_5::ret_code_t
		try_subscribe_event(
			subscription_info_t & subscription_info,
			event_subscriber_t & event_subscriber,
			so_5::throwing_strategy_t throwing_strategy );

		//! Наименование стадии.
		const stagepoint_t m_stagepoint;

		//! Агент, которому выдана привязка.
		so_5::rt::agent_t & m_agent;

		//! Слой mbapi, который создал данную привязку.
		mbapi_layer_impl_t & m_mbapi_layer;

		//! Замок для операция отсылки и подписки.
		ACE_RW_Thread_Mutex & m_lock;

		//! Таблица подписанных типов.
		subscribed_types_table_t m_subscribed_types_table;

		//! Mbox, для организации подписки.
		mutable so_5::rt::mbox_ref_t m_mbox;
};

//! Тип unique_ptr для stagepoint_bind_impl_t;
typedef std::unique_ptr< stagepoint_bind_impl_t >
	stagepoint_bind_impl_unique_ptr_t;

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
