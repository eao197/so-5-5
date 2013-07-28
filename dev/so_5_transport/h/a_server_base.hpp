/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Базовый класс агента выполняющего роль сервера.
*/

#if !defined( _SO_5_TRANSPORT__A_SERVER_BASE_HPP_ )
#define _SO_5_TRANSPORT__A_SERVER_BASE_HPP_

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/a_channel_base.hpp>

namespace so_5_transport
{

//
// a_server_base
//

//! Базовый класс агента выполняющего роль сервера.
/*!
	Определяет методы получения уведомлений о событиях канала,
	и делегирует обработку в hook-методы, которые могут быть переопределены в
	наследниках.
*/
class SO_5_TRANSPORT_TYPE a_server_base_t
	:
		public a_channel_base_t
{
		typedef a_channel_base_t base_type_t;

	public:
		a_server_base_t(
			//! Среда к которой привязывается агент.
			so_5::rt::so_environment_t & env,
			//! Mbox на который приходят уведомления о событиях с каналом.
			const so_5::rt::mbox_ref_t & notification_mbox );

		virtual ~a_server_base_t();

		virtual void
		so_define_agent();

		//! Обработка статуса создания серверного сокета.
		void
		evt_create_server_socket_result(
			const so_5::rt::event_data_t< msg_create_server_socket_result > &
				msg );

	protected:
		//! Hook-метод обработки статуса создания сокета.
		/*!
			Реализация по умолчанию ничего не далает,
			если установка серверного сокета прошла успешно,
			и дерегистрирует кооперацию, если установка
			серверного сокета не удалась.
		*/
		virtual void
		so_handle_create_server_socket_result(
			const msg_create_server_socket_result & msg );
};

} /* namespace so_5_transport */

#endif
