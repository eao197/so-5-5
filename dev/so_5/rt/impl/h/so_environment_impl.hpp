/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реализация среды исполнения so_5.
*/

#if !defined( _SO_5__RT__IMPL__SO_ENVIRONMENT_IMPL_HPP_ )
#define _SO_5__RT__IMPL__SO_ENVIRONMENT_IMPL_HPP_

#include <so_5/rt/impl/h/layer_core.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5/rt/impl/h/mbox_core.hpp>
#include <so_5/rt/impl/h/agent_core.hpp>
#include <so_5/rt/impl/h/disp_core.hpp>
#include <so_5/rt/impl/h/layer_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// so_environment_impl_t
//

//! Реализация среды so_5.
class so_environment_impl_t
{
	public:
		explicit so_environment_impl_t(
			const so_environment_params_t & so_environment_params,
			so_environment_t & public_so_environment );

		~so_environment_impl_t();


		//! \name Работа с mbox-ами.
		//! \{
		inline mbox_ref_t
		create_local_mbox()
		{
			return m_mbox_core->create_local_mbox();
		}

		inline mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & nonempty_name )
		{
			return m_mbox_core->create_local_mbox( nonempty_name );
		}

		inline mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & nonempty_name,
			//! Замок созданный пользователем
			std::unique_ptr< ACE_RW_Thread_Mutex >
				lock_ptr )
		{
			return m_mbox_core->create_local_mbox(
				nonempty_name,
				std::move( lock_ptr ) );
		}

		inline mbox_ref_t
		create_local_mbox(
			//! Замок созданный пользователем
			std::unique_ptr< ACE_RW_Thread_Mutex >
				lock_ptr )
		{
			return m_mbox_core->create_local_mbox( std::move( lock_ptr ) );
		}
		//! \}

		//! Создать мутекс для кооперации агентов.
		inline ACE_Thread_Mutex &
		create_agent_coop_mutex()
		{
			return m_agent_core.allocate_agent_coop_mutex();
		}

		//! Уничтожить мутекс для кооперации агентов.
		inline void
		destroy_agent_coop_mutex( ACE_Thread_Mutex & m )
		{
			return m_agent_core.deallocate_agent_coop_mutex( m );
		}

		//! Создать локальную очередь для агента.
		inline local_event_queue_unique_ptr_t
		create_local_queue()
		{
			return m_agent_core.create_local_queue();
		}

		//! \name Диспетчеры.
		//! \{

		//! Получить диспетчер по умолчанию.
		inline dispatcher_t &
		query_default_dispatcher()
		{
			return m_disp_core.query_default_dispatcher();
		}

		//! Получить диспетчер по имени.
		inline dispatcher_ref_t
		query_named_dispatcher(
			const std::string & disp_name )
		{
			return m_disp_core.query_named_dispatcher( disp_name );
		}

		//! Установить логер исключений.
		inline void
		install_exception_logger(
			event_exception_logger_unique_ptr_t && logger )
		{
			m_disp_core.install_exception_logger( std::move( logger ) );
		}

		//! Установить обработчик исключений.
		inline void
		install_exception_handler(
			event_exception_handler_unique_ptr_t && handler )
		{
			m_disp_core.install_exception_handler( std::move( handler ) );
		}
		//! \}

		//! \name Работа с кооперациями.
		//! \{

		//! Зарегистрировать кооперацию.
		ret_code_t
		register_coop(
			//! Кооперация, агентов которую надо зарегистрировать.
			agent_coop_unique_ptr_t agent_coop,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy )
		{
			return m_agent_core.register_coop(
				std::move( agent_coop ),
				throwing_strategy );
		}

		//! Дерегистрировать кооперацию.
		ret_code_t
		deregister_coop(
			//! Имя дерегистрируемой кооперации.
			const nonempty_name_t & name,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy )
		{
			return m_agent_core.deregister_coop(
				name, throwing_strategy );
		}

		//! Уведомить о готовности кооперации
		//! к окончательной дерегистрации.
		inline void
		ready_to_deregister_notify(
			agent_coop_t * coop )
		{
			m_agent_core.ready_to_deregister_notify( coop );
		}

		//! Окончательно дерегистрировать кооперацию.
		inline void
		final_deregister_coop(
			//! Имя дерегистрируемой кооперации.
			const std::string & coop_name )
		{
			m_agent_core.final_deregister_coop( coop_name );
		}
		//! \}

		//! \name Работа с таймерными событиями.
		//! \{

		//! Запланировать таймерное событие.
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Тип сообщения
			const rt::type_wrapper_t & type_wrapper,
			//! Сообщение
			const message_ref_t & msg,
			//! mbox на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед первой отправкой сообщений.
			unsigned int delay_msec,
			//! Период отправки сообщений, для отложенных сообщений
			//! period_msec == 0.
			unsigned int period_msec );

		//! Запланировать единичное таймерное событие,
		//! которое нельзя отменить.
		void
		single_timer(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Сообщение.
			const message_ref_t & msg,
			//! mbox на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед отправкой сообщений.
			unsigned int delay_msec );

		//! \}

		//! \name Работа слоями.
		//! \{
		so_layer_t *
		query_layer(
			const type_wrapper_t & type ) const;

		//! Добавить дополнительный слой.
		/*!
			\see layer_core_t::add_extra_layer().
		*/
		ret_code_t
		add_extra_layer(
			const type_wrapper_t & type,
			const so_layer_ref_t & layer,
			throwing_strategy_t throwing_strategy );
		//! \}

		//! \name Запуск, инициализация и остановка.
		//! \{
		ret_code_t
		run(
			so_environment_t & env,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );

		void
		stop();
		//! \}

		//! Получить ссылку на публичный экземпляр so_environment.
		so_environment_t &
		query_public_so_environment();

	private:
		//! Ядро подсистемы mbox-ов.
		mbox_core_ref_t m_mbox_core;

		//! Ядро подсистемы агентов.
		agent_core_t m_agent_core;

		//! Ядро подсистемы диспетчеров.
		disp_core_t m_disp_core;

		//! Ядро подсистемы слоев.
		layer_core_t m_layer_core;

		//! Ссылка на публичный экземпляр среды SO.
		so_environment_t & m_public_so_environment;

		//! Таймерная нить.
		so_5::timer_thread::timer_thread_unique_ptr_t
			m_timer_thread;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
