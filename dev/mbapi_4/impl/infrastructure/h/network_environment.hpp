/*
	MBAPI 4.
*/

/*!
	\file
	\brief Окружение mbapi-сети.
*/

#if !defined( _MBAPI_4__IMPL__INFRASTRUCTURE__NETWORK_ENVIRONMENT_HPP_ )
#define _MBAPI_4__IMPL__INFRASTRUCTURE__NETWORK_ENVIRONMENT_HPP_

#include <ace/RW_Thread_Mutex.h>

#include <so_5/rt/h/mbox_ref.hpp>

#include <mbapi_4/defs/h/mbapi_node_uid.hpp>

#include <mbapi_4/impl/infrastructure/h/availability_tables.hpp>
#include <mbapi_4/impl/infrastructure/h/channel_info.hpp>

namespace mbapi_4
{

namespace impl
{

namespace infrastructure
{

//
// network_environment_t
//

//! Сетевое окружение mbapi.
/*!
	Хранит в себе всю информацию о доступных конечных точках
	(таблица конечных точек)  и точках-стадиях (таблица точек-стадий)
	данного узла.

	\note Класс не является потоко безопасным, о синхронизации должен
	заботиться пользователь.
*/
class network_environment_t
{
	public:
		network_environment_t();

		//! Получить идентификатор mbapi-узла.
		inline const mbapi_node_uid_t &
		node_uid() const
		{
			return m_node_uid;
		}

		//! Добавить локальную конечную точку.
		/*!
			При создании конечной точки на данном mbapi-узле
			информация о ней должна быть добавлена в таблицу
			конечных точек независимо от того была ли она там или нет.
		*/
		void
		add_local_endpoint(
			const endpoint_stage_chain_t & endpoint_stage_chain );

		//! Изьять локальную конечную точку.
		void
		remove_local_endpoint(
			const endpoint_t & endpoint );

		//! Добавить локальную точку-стадию.
		/*!
			При создании точки-стадии на данном mbapi-узле
			информация о ней должна быть добавлена в таблицу
			точек-стадий независимо от того была ли она там или нет.
		*/
		void
		add_local_stagepoint(
			const stagepoint_t & stagepoint );

		//! Изьять локальную точку-стадию.
		void
		remove_local_stagepoint(
			const stagepoint_t & stagepoint );


		//! Обновить окружение в соответствии с информацией из заданного канала.
		void
		update_channel(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id,
			//! Идентификатор mbapi-узла.
			const mbapi_node_uid_t & node_uid,
			//! Mbox транспортного агента mbapi.
			const so_5::rt::mbox_ref_t & mbox,
			//! Таблица конечных точек канала.
			const available_endpoint_table_t & endpoints,
			//! Таблица точек-стадий канала.
			const available_stagepoint_table_t & stagepoints );

		//! Скопировать данные для таблиц конечных точек и стадий.
		void
		copy_tables_data(
			//! Таблица конечных точек.
			infrastructure::available_endpoint_table_t & endpoints,
			//! Таблица точек-стадий.
			infrastructure::available_stagepoint_table_t & stagepoints ) const;

		//! Изьять информацию о канале, когда канал разорван.
		void
		delete_channel(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id );

		endpoint_list_t
		query_endpoint_list() const;

		//! Сдвинуться к следуюшей стадии.
		/*!
			\return Если следующую стадию определить удалось,
			то вернет true, иначе false.
		*/
		bool
		shift_to_next_stage(
			//! Конечная точка, с которой отправлено сообщение.
			const endpoint_t & from,
			//! Конечная точка, на которую отправлено сообщение.
			const endpoint_t & to,
			//! Текущая стадия, которая будет изменена если
			//! следующая стадия будет найдена.
			stagepoint_t & current_stage ) const;

		//! Запросить mbox и channel_uid для канала,
		//! в котором виден заданный stagepoint.
		/*!
			\return true -  если такой stagepoint найден и его
			mbox и channel_id определены.
		*/
		bool
		define_channel_info(
			const stagepoint_t & stagepoint,
			so_5::rt::mbox_ref_t & mbox,
			channel_uid_wrapper_t & channel_id ) const;

		//! Функция для юнит-тестирования.
		//! \{
		inline available_endpoint_table_t
		utest_get_endpoint_table() const
		{
			return m_endpoints;
		}

		inline available_stagepoint_table_t
		utest_get_stagepoint_table() const
		{
			return m_stagepoints;
		}

		inline channel_info_table_t
		utest_get_channel_info_table() const
		{
			return m_channels_infos;
		}
		//! \}

	private:
		//! Найти информацию о канале.
		/*!
			Если канал не найден, то он добавляется
			в таблицу информации о каналах.
		*/
		channel_info_t &
		find_channel(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id,
			//! Идентификатор mbapi-узла.
			const mbapi_node_uid_t & node_uid,
			//! Mbox транспортного агента mbapi.
			const so_5::rt::mbox_ref_t & mbox );

		//! Найти информацию о канале.
		channel_info_t &
		find_channel(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id );

		//! Удалить информацию о заданных конечных точках.
		void
		remove_endpoints(
			const endpoint_list_t & endpoints_to_be_removed );

		//! Удалить информацию о заданных точках-стадиях.
		void
		remove_stagepoints(
			const stagepoint_list_t & stagepoints_to_be_removed );

		//! Синхронизировать данные с таблицей конечных точек канала.
		void
		sync_endpoint_table(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id,
			//! Информация о канале.
			channel_info_t & channel_info,
			//! Таблица конечных точек канала.
			const available_endpoint_table_t & endpoints );

		//! Синхронизировать данные с таблицей точек-стадий канала.
		void
		sync_stagepoint_table(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id,
			//! Информация о канале.
			channel_info_t & channel_info,
			//! Таблица точек-стадий канала.
			const available_stagepoint_table_t & stagepoints );

		//! Сделать копию таблицы конечных точек.
		void
		copy_endpoints_table(
			//! Таблица конечных точек.
			infrastructure::available_endpoint_table_t & endpoints ) const;

		//! Сделать копию таблицы точек-стадий.
		void
		copy_stagepoints_table(
			//! Таблица конечных точек.
			infrastructure::available_stagepoint_table_t & stagepoints ) const;

		//! Определить следуюющую стадию в цепочке стадий данной конечной точки.
		//! Стадии перебираются в прямом направлении.
		bool
		shift_to_next_stage_direct(
			const endpoint_t & ep,
			stagepoint_t & current_stage ) const;

		//! Сдвинуться к первой стадии в цепочке стадий данной конечной точки.
		bool
		shift_to_first_stage_direct(
			const endpoint_t & ep,
			stagepoint_t & current_stage ) const;

		//! Идентификатор mbapi-узла.
		const mbapi_node_uid_t m_node_uid;

		//! Таблица конечных точек.
		available_endpoint_table_t m_endpoints;

		//! Таблица точек-стадий.
		available_stagepoint_table_t m_stagepoints;

		//! Таблица с информацией о каналах.
		channel_info_table_t m_channels_infos;
};

} /* namespace infrastructure */

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
