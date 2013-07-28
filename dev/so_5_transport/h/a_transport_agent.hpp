/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Транспортные агенты: серверный и клиентский.
*/

#if !defined( _SO_5_TRANSPORT__A_TRANSPORT_AGENT_HPP_ )
#define _SO_5_TRANSPORT__A_TRANSPORT_AGENT_HPP_

#include <set>

#include <so_5_transport/h/declspec.hpp>

#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/channel_controller.hpp>

namespace so_5_transport
{

//
// a_transport_agent_t
//

//! Базовый класс транспортного агента.
/*!
	Ведет список контроллеров для действующих каналов, через которые инициирует
	закрытие канала, при дерегистрации кооперации.
*/
class SO_5_TRANSPORT_TYPE a_transport_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		/*!
			Создает собственный mbox и mbox нотификатора.
		*/
		a_transport_agent_t(
			so_5::rt::so_environment_t & env );

		virtual ~a_transport_agent_t();

		//! Подписывает агента на события канала.
		virtual void
		so_define_agent();

		//! Инициирует закрытие всех действующих каналов.
		virtual void
		so_evt_finish();

		//! Обработка получения нового подключения.
		void
		evt_channel_created(
			const so_5::rt::event_data_t< msg_channel_created > & msg );

		//! Обработка потери подключения.
		void
		evt_channel_lost(
			const so_5::rt::event_data_t< msg_channel_lost > & msg );

		//! Получить mbox для уведомлений о событиях канала.
		/*!
			При создании кооперации агентов транспортного канала
			агент наследник a_client_base_t или a_server_base_t
			(сервер или клиент), должны получить mbox, на который
			им будут отправляться сообщения о событиях канала.
		*/
		inline const so_5::rt::mbox_ref_t &
		query_notificator_mbox() const
		{
			return m_self_mbox;
		}

	protected:
		//! Дополнительная обработка
		//! события потеря канала.
		virtual void
		on_lost( const msg_channel_lost & msg );

		//! Состояние невозможности установить соединение.
		const so_5::rt::state_t m_failed_state;

		//! Собственный mbox.
		so_5::rt::mbox_ref_t m_self_mbox;

	private:

		//! Тип для списка установленных соединений.
		typedef std::set< channel_controller_ref_t >
			channel_controllers_set_t;

		//! Список установленных соединений.
		channel_controllers_set_t m_controllers;
};

//
// a_server_transport_agent_t
//

//! Серверный транспортный агент.
/*!
	В своем начальном событии -- so_evt_start(), пытается
	установить сервер. Результат создания сервера отсылается
	в виде сообщения msg_create_server_socket_result на mbox канала.
*/
class SO_5_TRANSPORT_TYPE a_server_transport_agent_t
	:
		public a_transport_agent_t
{
		typedef a_transport_agent_t base_type_t;

	public:
		a_server_transport_agent_t(
			so_5::rt::so_environment_t & env,
			acceptor_controller_unique_ptr_t acceptor_controller );

		virtual ~a_server_transport_agent_t();

		//! Начало работы серверного транспортного агента.
		/*!
			Производит попытку старатовть сервер через
			m_acceptor_controller, который инициализируется в конструкторе.
		*/
		virtual void
		so_evt_start();

		//! Событие завершения работы агента.
		/*!
			Уничтожает m_acceptor_controller,
			затем вызывает родительскую реализацию so_evt_finish().
		*/
		virtual void
		so_evt_finish();

	private:
		//! Контроллер канала.
		acceptor_controller_unique_ptr_t m_acceptor_controller;
};

//
// client_reconnect_params_t
//

//! Стратегия переподключения при разрыве связи клиентского транспортного агента.
struct SO_5_TRANSPORT_TYPE client_reconnect_params_t
{
	public:
		client_reconnect_params_t(
			//! Ожидание до следующего подключения
			//! в случае потери канала.
			unsigned int on_lost_reconnect_timeout = 1000,
			//! Ожидание до следующего подключения
			//! в случае если установить соединение не удалось.
			unsigned int on_failed_reconnect_timeout = 3000,
			//! Выполняти ли переподключение вообще?
			bool do_reconnect = true );

		~client_reconnect_params_t();

		//! Ожидание до следующего подключения
		//! в случае потери канала.
		unsigned int m_on_lost_reconnect_timeout;

		//! Ожидание до следующего подключения
		//! в случае если установить соединение не удалось.
		unsigned int m_on_failed_reconnect_timeout;

		//! Выполняти ли переподключение вообще?
		bool m_do_reconnect;
};

//
// a_client_transport_agent_t
//

//! Клиентсикй транспортный агент.
/*!
	В своем начальном событии -- so_evt_start(), пытается
	установить подключение к серверу.
	Если установить подключение не удается и переподключение
	возможно, то клиент начинает периодически пытаться подключиться
	к серверу.
*/
class SO_5_TRANSPORT_TYPE a_client_transport_agent_t
	:
		public a_transport_agent_t
{
		typedef a_transport_agent_t base_type_t;

	public:
		a_client_transport_agent_t(
			so_5::rt::so_environment_t & env,
			connector_controller_unique_ptr_t connector_controller,
			const client_reconnect_params_t & client_reconnect_params
				= client_reconnect_params_t() );

		virtual ~a_client_transport_agent_t();

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

		//! Уничтожает m_connector_controller,
		//! затем вызывает родительскую реализацию so_evt_finish().
		virtual void
		so_evt_finish();

		//! Сообщение для организации переподключений.
		struct SO_5_TRANSPORT_TYPE msg_reconnect
			:
				public so_5::rt::message_t
		{};

		//! Обработка сообщения о переподключении.
		void
		evt_reconnect(
			const so_5::rt::event_data_t< msg_reconnect > & );

	protected:
		//! Определяем дополнительную обработку
		//! потери канала, для того чтобы запланировать
		//! переподключение.
		virtual void
		on_lost( const msg_channel_lost & );

	private:
		//! Переподключиться.
		void
		do_reconnect();

		//! Выполнить попытку подключения.
		bool
		try_to_connect();

		//! Параметры переподключения.
		const client_reconnect_params_t m_client_reconnect_params;

		//! Контроллер канала.
		connector_controller_unique_ptr_t m_connector_controller;
};

} /* namespace so_5_transport */

#endif
