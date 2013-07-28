/*
	MBAPI 4.
*/

/*!
	\file
	\brief Обработчик mbapi-трафика данного канала.
*/

#if !defined( _MBAPI_4__COMM__IMPL_IOCHANNEL_DATA_PROCESSOR_HPP_ )
#define _MBAPI_4__COMM__IMPL_IOCHANNEL_DATA_PROCESSOR_HPP_

#include <ace/Time_Value.h>

#include <so_5_transport/h/ifaces.hpp>
#include <so_5_transport/h/io_trx.hpp>
#include <so_5_transport/h/channel_io.hpp>

#include <mbapi_4/defs/h/handshaking_params.hpp>
#include <mbapi_4/defs/h/mbapi_node_uid.hpp>
#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>

#include <mbapi_4/proto/h/sync_tables_info.hpp>
#include <mbapi_4/proto/h/send_msg_info.hpp>
#include <mbapi_4/proto/h/ping_info.hpp>
#include <mbapi_4/proto/h/handshake_info.hpp>

#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>

#include <mbapi_4/comm/impl/h/traffic_transformator.hpp>

#include <mbapi_4/impl/infrastructure/h/availability_tables.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

//
// channel_data_processor_t
//

//! Обработчик входящего/исходящего mbapi трафика.
class iochannel_data_processor_t
{
	public:
		iochannel_data_processor_t(
			//! Параметры процедуры handshake.
			const handshaking_params_t & handshaking_params,
			const so_5::rt::mbox_ref_t & mbox,
			const so_5_transport::channel_controller_ref_t & controller,
			const so_5_transport::channel_io_t & io );
		virtual ~iochannel_data_processor_t();

		inline const channel_uid_wrapper_t &
		channel_uid() const
		{
			return m_channel_uid;
		}

		//! Обработать входящий трафик
		void
		process_incoming(
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
			so_5_transport::input_trx_t & in_trx );

		void
		process_outgoing(
			//! Содержание сообщения.
			transmit_info_t & transmit_info );

		inline void
		close()
		{
			m_controller->close();
		}

		//! Синхронизировать таблицы конечных точек и точек-стадий.
		void
		sync_tables(
			const mbapi_4::impl::infrastructure::available_endpoint_table_t & available_endpoint_table,
			const mbapi_4::impl::infrastructure::available_stagepoint_table_t & available_stagepoint_table );

		//! Проверить является ли канала активным.
		/*!
			Канал считается не активным если в
			течении 5 секунд из него ничего не рипходило.

			Т.к. синхронизация таблиц конечных точек и точек стадий
			должна происходить с периодом 1 секунда, то
			если в течении 5 секунд их канала ничего не приходит,
			то значит что с той стороны что-то не так.
		*/
		bool
		is_active() const;

	protected:
		//! Записать сообщение в канал.
		void
		format(
			const proto::req_info_ptr_t & item );

		//! Обработать пакет входящих данных.
		/*!
			Данные в пакете должны быть уже преобарзованы.
			\return true - в случае если пакет был обработан.

			\note Реализация в данном классе обрабатывает только
			такие пакеты которые для клиента и сервера
			обрабатываются одинакого.
		*/
		virtual bool
		handle_request(
			const proto::req_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		//! Задать идентификатор узла с которым установлен канал.
		void
		set_mbapi_node_uid(
			const std::string & uid );

	private:
		//! Обработать запрос/ответ на приход сообщения.
		//! \{
		void
		handle(
			const proto::send_msg_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		//! Обработать ответ приход сообщения.
		void
		handle(
			const proto::send_msg_resp_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );
		//! \}

		//! Обработать запрос/ответ пинга.
		//! \{
		void
		handle(
			const proto::ping_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		void
		handle(
			const proto::ping_resp_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );
		//! \}

		//! Обработать запрос/ответ синхронизации таблиц.
		//! \{
		void
		handle(
			const proto::sync_tables_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		void
		handle(
			const proto::sync_tables_resp_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );
		//! \}

		//! Получить указатель на себя.
		const iochannel_data_processor_t *
		self_ptr() const;

	protected:
		//! Уникальный идентификатор канала,
		//! котороый генерируется в момент создания.
		const channel_uid_wrapper_t m_channel_uid;

		//! Идентификатор узла с которым установлен канал.
		mbapi_node_uid_t m_node_uid;

		const so_5::rt::mbox_ref_t & m_mbox;

		//! Контроллер канала.
		so_5_transport::channel_controller_ref_t m_controller;

		//! Потоки канала.
		so_5_transport::channel_io_t m_io;

		//! Параметры процедуры handshake.
		const handshaking_params_t & m_handshaking_params;

		//! Трансформатор трафика канала.
		traffic_transformator_unique_ptr_t m_traffic_transformator;

		//! Было ли произведено рукопожатие.
		bool m_handshaked;

		//! Время в которое пришли и были обработаны последние данные.
		ACE_Time_Value m_las_activity_timestamp;
};

//
// client_channel_data_processor_t
//

//! Обработчик клиентского подключения.
class client_channel_data_processor_t
	:
		public iochannel_data_processor_t
{
		typedef iochannel_data_processor_t base_type_t;
	public:
		client_channel_data_processor_t(
			const handshaking_params_t & handshaking_params,
			const so_5::rt::mbox_ref_t & mbox,
			const so_5_transport::channel_controller_ref_t & controller,
			const so_5_transport::channel_io_t & io );

		virtual ~client_channel_data_processor_t();

	protected:
		//! Обработать пакет входящих данных.
		/*!
			Данные в пакете должны быть уже преобарзованы.
			\return true - в случае если пакет был обработан.

			\note Реализация в данном классе проверяет
			было ли рукопожатие между сторонами, если
			нет, то обрабатывает только сообщение о рукопожатии,
			иначе передает обработку базовой реализации.
		*/
		virtual bool
		handle_request(
			const proto::req_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

	private:

		void
		handle(
			const proto::handshake_info_t & request );
};


//! Псевдоним unique_ptr для client_channel_data_processor_t
typedef std::unique_ptr<
		client_channel_data_processor_t >
	client_channel_data_processor_unique_ptr_t;

//! Тип умного указателя на client_channel_data_processor_t.
typedef std::shared_ptr< client_channel_data_processor_t >
	client_channel_data_processor_ptr_t;

//
// client_channel_data_processor_table_t
//

//! Хранилище обработчиков клиентских подключений.
class client_channel_data_processor_table_t
{
	public:
		//! Проверить есть ли клиентский канал с таким идентификатором.
		bool
		has_client_channel(
			const channel_uid_wrapper_t & channel_id ) const;

		//! Вставить клиентский канал.
		/*!
			\note Проверка что такой канал уже есть не делается.
		*/
		void
		insert_client_channel(
			const channel_uid_wrapper_t & channel_id,
			client_channel_data_processor_unique_ptr_t client_channel_data_processor );

		//! Удалить клиентский канал.
		void
		remove_client_channel(
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
			const channel_uid_wrapper_t & channel_id );

		void
		remove_all_client_channels(
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		//! Обработать входящие данные.
		void
		process_incoming(
			const channel_uid_wrapper_t & channel_id,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
			so_5_transport::input_trx_t & in_trx );

		//! Обработать отправку сообщения.
		void
		process_outgoing(
			transmit_info_t & transmit_info );

		void
		close(
			const channel_uid_wrapper_t & channel_id );

		void
		sync_tables(
			const mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		//! Проверить активность каналов.
		void
		check_activity();

		bool
		has_channels() const
		{
			return !m_clients.empty();
		}

	private:
		typedef std::map<
				const channel_uid_wrapper_t,
				client_channel_data_processor_ptr_t >
			processor_table_t;

		processor_table_t m_clients;
};

//
// server_channel_data_processor_t
//

//! Обработчик серверного подключения.
class server_channel_data_processor_t
	:
		public iochannel_data_processor_t
{
		typedef iochannel_data_processor_t base_type_t;
	public:
		server_channel_data_processor_t(
			const handshaking_params_t & handshaking_params,
			const so_5::rt::mbox_ref_t & mbox,
			const so_5_transport::channel_controller_ref_t & controller,
			const so_5_transport::channel_io_t & io );

		virtual ~server_channel_data_processor_t();

		//! Инициировать отправку рукопожатия.
		void
		initiate_handshake(
			const mbapi_node_uid_t & node_uid );

		//! Синхронизировать таблицы конечных точек и точек-стадий.
		void
		sync_tables(
			const mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

		void
		check_activity();

	protected:
		//! Обработать пакет входящих данных.
		/*!
			Данные в пакете должны быть уже преобарзованы.
			\return true - в случае если пакет был обработан.

			\note Реализация в данном классе проверяет
			было ли рукопожатие между сторонами, если
			нет, то обрабатывает только ответ о рукопожатии,
			иначе передает обработку базовой реализации.
		*/
		virtual bool
		handle_request(
			const proto::req_info_t & request,
			mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer );

	private:
		void
		handle(
			const proto::handshake_resp_info_t & request );
};

} /* namespace impl */

} /* namespace comm */

} /* namespace mbapi_4 */

#endif
