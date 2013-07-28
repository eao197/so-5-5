/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Обобщенная реализация Svc_Handler.
*/

#if !defined( _SO_5_TRANSPORT__SOCKET__IMPL__SVC_HANDLER_HPP_ )
#define _SO_5_TRANSPORT__SOCKET__IMPL__SVC_HANDLER_HPP_

#include <cassert>

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

#include <cpp_util_2/h/lexcast.hpp>

#include <so_5_transport/h/ret_code.hpp>
#include <so_5_transport/h/raw_block.hpp>
#include <so_5_transport/h/scattered_block_stream.hpp>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/mbox_ref.hpp>

#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/channel_params.hpp>

#include <so_5_transport/socket/impl/h/io_timestamps.hpp>

// #include <so_5_transport/impl/h/block_stream_notifiers.hpp>
// #include <so_5_transport/impl/h/generic_controller.hpp>
// #include <so_5_transport/impl/h/helpers.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// last_problem_t
//

//! Класс для хранения сообщения о последней ошибке.
class last_problem_t
{
	public:
		//! Установить новое сообщение об ошибке
		//! и вернуть c_str() этой ошибки.
		const char *
		operator () ( const std::string & problem );

		const std::string &
		str() const;

	private:
		std::string m_last_problem;
};


//
// svc_handler_t
//

//! Реализация ACE_Svc_Handler для работы с сетевым соединением.
/*!
	Отвечает за зачитывание данных и их вставку во входящий поток,
	а также за запись данных в сокет.
*/
class svc_handler_t
	:
		public ACE_Svc_Handler< ACE_SOCK_Stream, ACE_NULL_SYNCH >
{
		//! Псевдоним базового типа.
		typedef ACE_Svc_Handler< ACE_SOCK_Stream, ACE_NULL_SYNCH > base_type_t;

	public:
		virtual ~svc_handler_t();

		svc_handler_t();

		//! Инициализирующий конструктор.
		svc_handler_t(
			//! Реактор, на котором должен работать канал.
			ACE_Reactor * reactor,
			//! Идентификатор канала.
			channel_id_t channel_id,
			//! Mbox транспортного агента.
			const so_5::rt::mbox_ref_t & ta_mbox,
			//! Параметры канала.
			const channel_params_t & channel_params );

		//! \name Реализация унаследованных от Svc_Handler методов.
		//! \{

		/*!
			\brief Реакция на успешное установление соединения.

			Этот метод вызывается connector-ом/acceptor-ом после того, как
			процедура установления соединения будет завершена.

			Полностью переопределяет метод open() базового класса:
			 - если задан реактор, то регистрирует себя в реакторе с нулевой
			  маской (т.е. без определения готовности канала к чтению или
			  записи).
		*/
		virtual int
		open( void * );

		/*!
			\brief Реакция на закрытие канала или на неудачную попытку
				создания канала.

			Этот метод вызывается connector-ом/acceptor-ом при неудачной
			попытке создания канала. Либо из метода close() при разрушении
			объекта.

			Если вызов выполняется когда объект находится в состоянии
			awaiting_completion, то владельцу канала отсылается уведомление
			о неудачной попытке создания канала.

			Если вызов выполняется когда объект находится в состоянии
			established, то владельцу канала отсылается уведомление
			о потере канала.
		 */
		virtual int
		handle_close(
			ACE_HANDLE handle,
			ACE_Reactor_Mask mask );

		/*!
			\brief Реакция на готовность канала к операциям чтения данных.
		 */
		int
		handle_input( ACE_HANDLE );

		/*!
			\brief Реакция на готовность канала к операциям записи данных.
		*/
		virtual int
		handle_output( ACE_HANDLE );

		/*!
			\brief Реакция на срабатывание таймера контроля за состоянием канала.

			Проверяет время блокированности канала и время после последней
			успешной записи.
		*/
		virtual int
		handle_timeout(
			const ACE_Time_Value & current_time,
			const void * );

		/*!
			Вызов этого метода инициируется из
			controller_close(), который вызван не на нити реактора, для
			того, чтобы уничтожить svc_handler на нити реактора.

			Если вызов происходит из-за controller_close(), то
			для svc_handler выставляется состояние CONNECTION_MUST_BE_DESTROYED
			и возвращается -1 для уничтожение svc_handler-а.

			В противном случае ничего не делается и возвращается 0.
		*/
		virtual int
		handle_exception( ACE_HANDLE fd );

		//! Запланировать отправку данных.
		void
		output_notify();

		//! Запланировать отправку данных.
		void
		block_input();

		//! Запланировать отправку данных.
		void
		unblock_input();

		void
		enforce_input_detection();

		int
		initiate_except();

	private:
		/*!
			Идентификатор канала.
			Назначается acceptor_controller-ом или connector_controller-ом.
		*/
		const channel_id_t m_channel_id;

		//! Mbox транспортного агента.
		/*!
			Имеет значение только до отработки open.
			Его необходимо передать уведомителю исходящего канала.
		*/
		so_5::rt::mbox_ref_t m_ta_mbox;

		/*!
			Параметры канала.
			Инициализируется в конструкторе.
		*/
		const channel_params_t m_channel_params;

		//! Временные отметки операций чтения/записи.
		io_timestamps_t m_io_timestamps;

		//! Контроллер канала.
		channel_controller_ref_t m_controller;

		//! Потоки для вычитанных данных и для записи данных.
		scattered_block_stream_ref_t m_input_stream;
		scattered_block_stream_ref_t m_output_stream;

		/*!
			Идентификатор таймерного событий, полученный от
			ACE_Reactor::schedule_timer.
			При закрытии канала таймерное событие отменяется
			по этому идентификатору.
		*/
		long m_timer_id;

		volatile bool m_enforce_input_detection;
		volatile bool m_is_input_blocked;
		volatile bool m_is_output_blocked;

		std::unique_ptr< block_stream_notifiers::flush_notifier_t >
			m_output_flush_notifier;
		std::unique_ptr< block_stream_notifiers::free_blocks_availability_notifier_t >
			m_input_free_blocks_availability_notifier;
		std::unique_ptr< block_stream_notifiers::stream_operation_notifier_t >
			m_stream_operation_notifier;

		//! Описание последней серьезной проблемы.
		last_problem_t m_last_problem;

		//! Создать каналы.
		void
		create_streams();

		//! Зачитать данные.
		int
		read_channel(
			bool & channel_closed_on_remote_host );

		//! Записать данные.
		int
		write_channel();
};

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

#endif
