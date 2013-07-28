/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Служебные сообщения транспортного слоя.
*/

#if !defined( _SO_5_TRANSPORT__MESSAGES_HPP_ )
#define _SO_5_TRANSPORT__MESSAGES_HPP_

#include <memory>
#include <string>

#include <so_5_transport/h/declspec.hpp>

#include <so_5/rt/h/message.hpp>

#include <so_5_transport/h/ifaces.hpp>
#include <so_5_transport/h/channel_io.hpp>

namespace so_5_transport
{

//
// msg_create_server_socket_result
//

//! Статус создания серверного сокета.
struct SO_5_TRANSPORT_TYPE msg_create_server_socket_result
	:
		public so_5::rt::message_t
{
	//! Успешно ли завершилось создание серверного сокета.
	bool m_succeed;

	msg_create_server_socket_result();
	msg_create_server_socket_result(
		bool succeed );
};


//
// msg_channel_created
//

/*!
	\brief Сообщение об успешном создании транспортного канала.

	Отсылается объектом-connector-ом или объектом-acceptor-ом
	при успешном установлении соединения с удаленной стороной.

	\note Получатель сообщения должен быть только 1.
 */
struct SO_5_TRANSPORT_TYPE msg_channel_created
	:
		public so_5::rt::message_t
{
	//! Идентификатор данного канала.
	channel_id_t m_channel_id;

	//! Создатель потоков ввода-вывода.
	channel_io_t m_io;

	//! Контроллер канала.
	mutable channel_controller_ref_t m_controller;

	msg_channel_created();
	msg_channel_created(
		const channel_id_t & id,
		const channel_io_t & io,
		const channel_controller_ref_t & controller );

	virtual ~msg_channel_created();

};

//
// msg_channel_failed
//

/*!
	\brief Сообщение о неудачном создании транспортного канала.

	Отсылается объектом-connector-ом или объектом-acceptor-ом
	после неудачной попытки установления соединения с удаленной стороной.
*/
struct SO_5_TRANSPORT_TYPE msg_channel_failed
	:
		public so_5::rt::message_t
{
	std::string m_reason;

	msg_channel_failed();
	msg_channel_failed( const std::string & reason );

	virtual ~msg_channel_failed();

};

//
// msg_channel_lost
//

/*!
	\brief Сообщение о закрытии транспортного канала.

	Отсылается при обнаружении того,
	что канал оказался закрытым или разорванным.
 */
struct SO_5_TRANSPORT_TYPE msg_channel_lost
	:
		public so_5::rt::message_t
{
	//! Идентификатор потерянного канала.
	channel_id_t m_channel_id;

	//! Контроллер канала.
	mutable channel_controller_ref_t m_controller;

	/*!
		\brief Текстовое описание причины закрытия.

		\note Может оказаться, что не во всех случаях причина неудачи
		будет известна. Поэтому иногда это поле может быть пустым.
	 */
	const std::string m_reason;

	msg_channel_lost();
	msg_channel_lost(
		const channel_id_t & id,
		const std::string & reason,
		const channel_controller_ref_t & controller );

	virtual ~msg_channel_lost();
};

//
// msg_incoming_package
//

//! Сообщение-уведомление о поступлении нового пакета данных.
struct SO_5_TRANSPORT_TYPE msg_incoming_package
	:
		public so_5::rt::message_t
{
	//! Идентификатор канала.
	channel_id_t m_channel_id;

	msg_incoming_package();

	msg_incoming_package(
		channel_id_t channel_id,
		const channel_io_t & io );

	virtual ~msg_incoming_package();

	//! \name Интерфейс для создания потоков чтения/записи.
	//! \{

	//! Начать транзакцию чтения.
	/*!
		Начать транзакцию чтения можно только 1 раз.
	*/
	input_trx_unique_ptr_t
	begin_input_trx() const;

	//! Создать поток для записи.
	output_trx_unique_ptr_t
	begin_output_trx(
		//! Время которое можно потратить на ожидание
		//! свободных блоков в случае их отсутствия или исчерпания
		//! в ходе записи.
		const ACE_Time_Value &
			free_blocks_awaiting_timeout = ACE_Time_Value::zero ) const;
	//! \}

	private:
		//! Создатель потоков ввода-вывода.
		channel_io_t m_io;

		//! Имела ли место транзакция чтения данных.
		/*!
			Если пользователь не начинал транзакцию чтения данных,
			тогда для того чтобы приход новых данных приводил к
			отправке уведомления о входящем пакете.
		*/
		mutable bool m_input_transaction_took_place;
};

} /* namespace so_5_transport */

#endif
