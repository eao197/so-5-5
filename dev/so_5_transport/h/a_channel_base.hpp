/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Базовый класс агента взаимодйствующего с каналом.

	Определяет обработчики событий канала, подписывает агента на события
	и делегирует обработку hook-методам.
*/

#if !defined( _SO_5_TRANSPORT__A_CHANNEL_BASE_HPP_ )
#define _SO_5_TRANSPORT__A_CHANNEL_BASE_HPP_

#include <set>

#include <so_5/rt/h/agent.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/messages.hpp>

namespace so_5_transport
{

// Эти классы должны быть друзьями.
class a_client_base_t;
class a_server_base_t;

//
// a_channel_base_t
//

//!  Базовый класс канального агента.
/*!
	Служит базовым классом канальных агентов
	Определяет методы получения уведомлений о событиях канала,
	и делегирует их обработку hook-методам, которые должны быть переопределены в
	наследниках.

	Наследование от данного класса избавляет от необходимости
	самому подписывать агента на события канала и хранить mbox для
	уведомлений. Если наследник определяет дополнительные состояния
	и хочет получать уведомления о событиях с каналом, то можно
	добавить состояние в список состояний для подписки на события канала.
*/
class SO_5_TRANSPORT_TYPE a_channel_base_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;
		friend class a_server_base_t;
		friend class a_client_base_t;

	public:
		a_channel_base_t(
			so_5::rt::so_environment_t & env,
			//! Mbox на который будут приходить уведомления о событиях канала.
			const so_5::rt::mbox_ref_t & notification_mbox );
		virtual ~a_channel_base_t();

		//! Определяет подписку агента на события канала.
		virtual void
		so_define_agent();

		//! Обработка установки нового соединения.
		void
		evt_channel_created(
			const so_5::rt::event_data_t< msg_channel_created > & msg );

		//! Обработка ошибки установки нового соединения.
		void
		evt_channel_failed(
			const so_5::rt::event_data_t< msg_channel_failed > & msg );

		//! Обработка потери соединения.
		void
		evt_channel_lost(
			const so_5::rt::event_data_t< msg_channel_lost > & msg );

		//! Обработка уведомления о входящем пакете.
		void
		evt_incoming_package(
			const so_5::rt::event_data_t<
				so_5_transport::msg_incoming_package > & msg );

	protected:
		//! Добавить состояние для подписки на события канала.
		/*!
			Если наследник определяет дополнительные состояния
			и надо чтобы события канала обрабатывались в этих
			дополнительных состояниях, то через so_add_state_for_channel_events
			их можно добавить в список состояний для которых
			обрабатываются события канала. Например:
			\code
				class my_client_t : public so_5_transport::a_channel_base_t
				{
					typedef so_5_transport::a_channel_base_t base_type_t;

					// Версия протокола 1.
					const so_5::state_t m_protocol_v1;
					// Версия протокола 2.
					const so_5::state_t m_protocol_v2;

					public:
						my_client_t(
							so_5::rt::so_environment_t & env,
							const so_5::rt::mbox_ref_t & notification_mbox )
							:
								base_type_t( env )
								m_protocol_v1( self_ptr() ),
								m_protocol_v2( self_ptr() )
						{
							// В не зависимости от того какая версия протокола
							// будет использована события канала надо обрабатывать.
							so_add_state_for_channel_events( m_protocol_v1 );
							so_add_state_for_channel_events( m_protocol_v2 );
						}

						virtual void
						so_define_agent()
						{
							// Подписки базового класса.
							base_type_t::so_define_agent();

							so_subscribe( query_interface_mbox() )
								.in( m_protocol_v1 )
								.event( &my_client_t::evt_msg_1 );

							so_subscribe( query_interface_mbox() )
								.in( m_protocol_v2 )
								.event( &my_client_t::evt_msg_1 );

							// ...

							so_subscribe( query_interface_mbox() )
								.in( m_protocol_v1 )
								.event( &my_client_t::evt_msg_N );

							so_subscribe( query_interface_mbox() )
								.in( m_protocol_v2 )
								.event( &my_client_t::evt_msg_N );

							// Во второй версии добавлена поддержка пинга.

							so_subscribe( query_interface_mbox() )
								.in( m_protocol_v2 )
								.event( &my_client_t::evt_ping );

							// ...
						}

						// ...
				};
			\endcode

			\note Добавление состояний имеет смысл только
			до того момента как агент будет подписан на сообщения.
		*/
		void
		so_add_state_for_channel_events(
			const so_5::rt::state_t & state );

		//! Hook-методы обработки событий канала.
		//! \{

		//! Обработка появления нового клиента.
		virtual void
		so_handle_client_connected(
			const msg_channel_created & msg ) = 0;

		//! Обработка при неудачном подключении нового клиента.
		virtual void
		so_handle_client_failed(
			const msg_channel_failed & msg ) = 0;

		//! Обработка отключения клиента.
		virtual void
		so_handle_client_disconnected(
			const msg_channel_lost & msg ) = 0;

		//! Обработки очередного пакета клиента.
		virtual void
		so_handle_incoming_package(
			const so_5_transport::msg_incoming_package & msg ) = 0;
		//! \}

		//! Получить mbox на который приходят уведомлени
		//! о событиях канала.
		inline so_5::rt::mbox_ref_t &
		query_notification_mbox()
		{
			return m_notification_mbox;
		}

	private:
		//! Тип списка состояний.
		typedef std::set< const so_5::rt::state_t * > state_set_t;

		//! Список состояний для которых подписывать события канала.
		state_set_t m_states_listeninig_channel_events;

		//! Mbox на который приходят уведомлени
		//! о событиях канала.
		so_5::rt::mbox_ref_t m_notification_mbox;
};

} /* namespace so_5_transport */

#endif
