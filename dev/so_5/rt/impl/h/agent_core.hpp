/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс для обеспечения агентов и их коопераций мьютексами.
*/

#if !defined( _SO_5__RT__IMPL__AGENT_CORE_HPP_ )
#define _SO_5__RT__IMPL__AGENT_CORE_HPP_

#include <memory>
#include <string>
#include <map>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/util/h/mutex_pool.hpp>

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/coop_listener.hpp>

#include <so_5/rt/impl/h/local_event_queue.hpp>

#include <so_5/rt/impl/coop_dereg/h/coop_dereg_executor_thread.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// agent_core_t
//

//! Класс, для обеспечения агентов мьютексами.
class agent_core_t
{
		agent_core_t( const agent_core_t & );
		void
		operator = ( const agent_core_t & );

	public:
		explicit agent_core_t(
			//! Среда SObjectizer.
			so_environment_t & so_environment_impl,
			//! Размер пула мутексов для коопераций.
			unsigned int agent_coop_mutex_pool,
			//! Размер пула мутексов для локальных очередей агентов.
			unsigned int agent_queue_mutex_pool_size,
			//! Слущатель действий над кооперациями.
			coop_listener_unique_ptr_t coop_listener );

		~agent_core_t();

		//! Инициализировать ядро.
		void
		start();

		//! Инициировать остановку.
		void
		shutdown();

		//! Ожидать окончания.
		void
		wait();

		//! Операции с мутексом кооперпаций.
		//! \{
		//! Взять мутекс в пользование.
		ACE_Thread_Mutex &
		allocate_agent_coop_mutex();

		//! Отказаться от использования мутекса.
		void
		deallocate_agent_coop_mutex(
			ACE_Thread_Mutex & m );
		//! \}

		//! Создать локальную очередь для агента.
		local_event_queue_unique_ptr_t
		create_local_queue();

		//! Зарегистрировать кооперацию.
		ret_code_t
		register_coop(
			//! Кооперация, агентов которую надо зарегистрировать.
			agent_coop_unique_ptr_t agent_coop,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );

		//! Дерегистрировать кооперацию.
		ret_code_t
		deregister_coop(
			//! Имя дерегистрируемой кооперации.
			const nonempty_name_t & name,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );


		//! Уведомить о готовности кооперации
		//! к окончательной дерегистрации.
		void
		ready_to_deregister_notify(
			agent_coop_t * coop );

		//! Окончательно дерегистрировать кооперацию.
		void
		final_deregister_coop(
			//! Имя дерегистрируемой кооперации.
			/*!
				\note Параметр должен быть получен копией,
				т.к. к методу обращается сам обхект кооперации,
				а в результате работы он уничтожается. Поэтому,
				принимать ссылку на аттрибут объекта, который будет
				уничтожен - не корректно.
			*/
			const std::string coop_name );

		//! Инициировать начало дерегистрации.
		void
		start_deregistration();

		//! Ожидать сигнала о начале дерегистрации.
		void
		wait_for_start_deregistration();

		//! Дерегистрировать все кооперации.
		/*!
			Когда завершается работа SO, то все кооперации
			дерегистрируются.
		*/
		void
		deregister_all_coop();

		//! Ожидать пока все кооперации не буду тдерегистрированы.
		void
		wait_all_coop_to_deregister();

	private:
		//! Тип карты для зарегистрированных коопераций.
		typedef std::map<
				std::string,
				agent_coop_ref_t >
			coop_map_t;

		//! Метод для дерегистрации кооперации с std::for_each.
		static void
		coop_undefine_all_agents( agent_core_t::coop_map_t::value_type & coop );

		so_environment_t & m_so_environment;

		//! Пул мутексов для коопераций агентов.
		util::mutex_pool_t< ACE_Thread_Mutex > m_agent_coop_mutex_pool;

		//! Пул мутексов для очередей событий агентов.
		util::mutex_pool_t< ACE_Thread_Mutex > m_agent_queue_mutex_pool;

		//! Замок на операции с кооперациями.
		ACE_Thread_Mutex m_coop_operations_lock;

		//! Условная переменная, что дерегистрация начата.
		ACE_Condition_Thread_Mutex m_deregistration_started_cond;

		//! Условная переменная, что дерегистрация завершена.
		ACE_Condition_Thread_Mutex m_deregistration_finished_cond;

		//! Флаг о начале дерегистрации всех коопераций.
		bool m_deregistration_started;

		//! Зарегистрированные кооперации.
		coop_map_t m_registered_coop;

		//! Кооперации, которые помечены для дерегистрации.
		coop_map_t m_deregistered_coop;

		//! Нить окончательной дерегистрации коопераций.
		coop_dereg::coop_dereg_executor_thread_t m_coop_dereg_executor;

		//! Слущатель действий над кооперациями.
		coop_listener_unique_ptr_t m_coop_listener;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
