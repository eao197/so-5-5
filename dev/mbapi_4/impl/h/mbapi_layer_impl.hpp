/*
	MBAPI 4.
*/

/*!
	\file
	\brief Реализация слоя работы с mbapi_4.
*/

#if !defined( _MBAPI_4__IMPL__MBAPI_LAYER_IMPL_HPP_ )
#define _MBAPI_4__IMPL__MBAPI_LAYER_IMPL_HPP_

#include <vector>

#include <ace/RW_Thread_Mutex.h>

#include <oess_2/stdsn/h/serializable.hpp>

#include <so_5/rt/h/so_environment.hpp>

#include <so_5/util/h/mutex_pool.hpp>

#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/mbox.hpp>

#include <mbapi_4/defs/h/mbapi_node_uid.hpp>
#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>

#include <mbapi_4/impl/h/types.hpp>

#include <mbapi_4/impl/h/stagepoint_bind_impl.hpp>
#include <mbapi_4/impl/infrastructure/h/network_environment.hpp>

namespace mbapi_4
{

class mbapi_layer_t;

namespace impl
{

//
// bind_section_t
//

//! Группировка данных для работы сертификатов.
struct bind_section_t
{
	bind_section_t()
		:
			m_bind_mutex_pool( 32 )
	{}

	//! Замок для операций с сертификатами.
	ACE_RW_Thread_Mutex m_lock;

	//! Тип для таблицы выданных сертификатов.
	typedef std::map< stagepoint_t, stagepoint_bind_impl_t * >
		bind_table_t;

	//! Таблица выданных сертификатов.
	bind_table_t m_bind_table;

	//! Пул мутексов для работы сертификатов.
	so_5::util::mutex_pool_t< ACE_RW_Thread_Mutex > m_bind_mutex_pool;
};

//
// network_section_t
//

//! Секция для работы с mbapi-сетью.
struct network_section_t
{
	//! Замок для проведения операций изменения структуры mbapi-сети.
	mutable ACE_RW_Thread_Mutex m_lock;

	//! Окружение mbapi-сети.
	infrastructure::network_environment_t m_netowork_environment;
};

//
// types_repositories_section_t
//

//! Секция для работы с репозиториями типов.
struct types_repositories_section_t
{
	void
	add_repository(
		const oess_2::stdsn::types_repository_t & types_repository );

	void
	remove_repository(
		const oess_2::stdsn::types_repository_t & types_repository );

	//! Хранилище типов.
	types_repositories_list_t m_types_repositories;

	//! Замок для работы репозиториев.
	ACE_RW_Thread_Mutex m_lock;
};

//
// mbapi_layer_impl_t
//

//! Реализация слоя работы с mbapi_4.
class mbapi_layer_impl_t
{
	public:
		mbapi_layer_impl_t(
			//! Среда SO.
			so_5::rt::so_environment_t & so_environment,
			//! Слой mbapi.
			mbapi_layer_t & mbapi_layer );

		~mbapi_layer_impl_t();

		inline so_5::rt::so_environment_t &
		so_environment()
		{
			return m_so_environment;
		}

		inline mbapi_layer_t &
		mbapi_layer()
		{
			return m_mbapi_layer;
		}

		//! Получить идентификатор mbapi-узла.
		inline const mbapi_node_uid_t &
		node_uid() const
		{
			return m_network_section
				.m_netowork_environment
					.node_uid();
		}

		//! Добавление и удаление репозиториев.
		//! \{
		inline void
		add_repository(
			const oess_2::stdsn::types_repository_t & types_repository )
		{
			m_types_repositories_section.add_repository( types_repository );
		}

		void
		remove_repository(
			const oess_2::stdsn::types_repository_t & types_repository )
		{
			m_types_repositories_section.remove_repository( types_repository );
		}
		//! \}

		//! Работа с сертификатами.
		//! \{
		inline ACE_RW_Thread_Mutex &
		allocate_mutex()
		{
			return m_bind_section
				.m_bind_mutex_pool
					.allocate_mutex();
		}

		inline void
		deallocate_mutex( ACE_RW_Thread_Mutex & m )
		{
			m_bind_section
				.m_bind_mutex_pool
					.deallocate_mutex( m );
		}

		//! Получить сертификат для заданной точки-стадии.
		//! Если сертификат для данной точки-стадии выдан,
		//! то возвращает нулевой указатель.
		stagepoint_bind_impl_unique_ptr_t
		create_bind(
			//! Наименование точки-стадии.
			const stagepoint_t & stagepoint,
			//! Агент, которому выдается сертификат.
			so_5::rt::agent_t & agent );

		//! Получить сертификат для заданной конечной точки.
		//! Если сертификат для данной конечной точки выдан,
		//! то возвращает нулевой указатель.
		stagepoint_bind_impl_unique_ptr_t
		create_bind(
			//! Наименование конечной точки и ее стадий.
			const endpoint_stage_chain_t & endpoint_stage_chain,
			//! Агент, которому выдается сертификат.
			so_5::rt::agent_t & agent );

		//! Исключить сертификат для данной точки-стадии из таблицы выданных
		//! сертификатов.
		void
		remove_bind(
			const stagepoint_t & stagepoint );
		//! \}

		//! Работа с сетевым окружением mbapi.
		//! \{

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
			const infrastructure::available_endpoint_table_t & endpoints,
			//! Таблица точек-стадий канала.
			const infrastructure::available_stagepoint_table_t & stagepoints );

		//! Заполнить данные для синхронизации с заданным каналом.
		/*!
			Делает копию своих таблиц конечных точек и точек-стадий.
		*/
		void
		copy_tables_data(
			//! Таблица конечных точек.
			infrastructure::available_endpoint_table_t & endpoints,
			//! Таблица точек-стадий.
			infrastructure::available_stagepoint_table_t & stagepoints ) const;

		//! Удаление канала.
		void
		delete_channel(
			//! Идентификатор канала.
			const channel_uid_wrapper_t & channel_id );

		//! Получить список доступных endpoint-ов.
		endpoint_list_t
		query_endpoint_list() const;

		//! \}

		//! Передача сообщений.
		//! \{

		//! Передать сообщение из канала.
		void
		transmit_message(
			comm::transmit_info_unique_ptr_t transmit_info );

		//! Передать бинарное сообщение.
		/*!
			Отличатеся от transmit_message(), тем что как и в случае
			local_transmit_info, в m_current_stage записано
			значение не текущего приемника, а отправителя.
		*/
		void
		transmit_banary_message(
			comm::transmit_info_unique_ptr_t transmit_info );

		//! Передать сообщение из данного mbapi-узла.
		void
		transmit_message(
			comm::local_transmit_info_unique_ptr_t local_transmit_info );
		//! \}

	private:
		//! Создать transmit_info_t из local_transmit_info_t.
		comm::transmit_info_unique_ptr_t
		create_transmit_info(
			const comm::local_transmit_info_t & local_transmit_info ) const;

		//! Среда SObjectizer.
		so_5::rt::so_environment_t & m_so_environment;

		//! Слой mbapi.
		mbapi_layer_t & m_mbapi_layer;

		//! Данные для работы слоя.
		/*!
			Каждая секция имеет замок, для синхрнизации работы
			над своими данными. Для предотвращения ошибок
			и появления deadlock-ов, порядок захвата замков,
			в случае необходимости захватывать сразу несколько
			должен быть следующий:
				\li m_bind_section.m_lock;
				\li m_network_section.m_lock;
				\li m_types_repositories_section.m_lock.
		*/
		//! \{
		//! Данные для работы с сертификатами.
		bind_section_t m_bind_section;

		//! Данные для работы с окружением mbapi-сети.
		network_section_t m_network_section;

		//! Данные для работы с репозиториями типов.
		types_repositories_section_t m_types_repositories_section;
		//! \}
};

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
