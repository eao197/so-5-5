/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реализация таймерной нити на основе
		ACE_Thread_Timer_Queue_Adapter.
*/

#if !defined( _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__TIMER_THREAD_HPP_ )
#define _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__TIMER_THREAD_HPP_

#include <memory>
#include <map>

#include <ace/Timer_Queue_Adapters.h>
#include <ace/Timer_List.h>

#include <so_5/timer_thread/h/timer_thread.hpp>

#include <so_5/timer_thread/ace_timer_queue_adapter/impl/h/event_handler.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

namespace impl
{

//
// timer_thread_t
//

/*!
	\brief Реализация таймерной нити на основе
	ACE_Thread_Timer_Queue_Adapter.
 */
class timer_thread_t
	:
		public so_5::timer_thread::timer_thread_t,
		public timer_act_utilizator_t
{
		//! Псевдоним для базового типа.
		typedef so_5::timer_thread::timer_thread_t base_type_t;
	public:

		//! Вспомогательный класс для хранения
		//! таймерных идентификаторов в ACE и в SObjectizer.
		struct timer_keys_t
		{
			timer_keys_t( timer_id_t timer_id, long ace_id )
				:
					m_timer_id( timer_id ),
					m_ace_id( ace_id )
			{}

			timer_id_t	m_timer_id;
			long		m_ace_id;
		};

		//! Конструктор.
		timer_thread_t();

		virtual ~timer_thread_t();

		/*!
		 * \name Реализация интерфейса so_5::timer_thread::timer_thread_t.
		 * \{
		 */

		//! Запустить нить таймера.
		/*!
		 * Вызывает ACE_Thread_Timer_Queue_Adapter::activate().
		 */
		virtual ret_code_t
		start();

		//! Дать сигнал нити таймера завершить работу.
		/*!
		 * Вызывает ACE_Thread_Timer_Queue_Adapter::deactivate().
		 */
		virtual void
		shutdown();

		//! Ожидать полного завершения работы нити таймера.
		/*!
		 * Ожидает на ACE_Thread_Timer_Queue_Adapter::wait().
		 */
		virtual void
		wait();

		//! Поставить отложенное или переодическое сообщение в очередь.
		/*!
		 * Назначает данной заявке собственный ID и создает Timer ACT.
		 * Регистрирует заявку в ACE_Thread_Timer_Queue_Adapter и сохраняет
		 * в m_agent_demands описание этой заявки.
		 *
		 * \note Блокирует таймерную нить.
		 */
		virtual timer_id_t
		schedule_act(
			timer_act_unique_ptr_t & timer_act );

		//! Отменить периодическое сообщение.
		virtual void
		cancel_act(
			timer_id_t msg_id );

		/*!
		 * \}
		 */

		//! Быстрая отмена события, без дополнительного захвата мутекса
		//! и без удаляения из мапа.
		//! \note Применяется только в конце при завершении
		//! таймерной нити.
		virtual void
		quick_cancel_act(
			const timer_keys_t & timer_keys );

		/*!
		 * \name Реализация интерфейса timer_act_utilizator_t.
		 * \{
		 */

		//! При необходимости утилизировать таймерное действие.
		virtual void
		utilize_timer_act(
			timer_act_t * timer_act );

		/*!
		 * \}
		 */

	private:
		//! Удалить все имеющиеся в таймере события.
		//! \note Применяется только в конце при завершении
		//! таймерной нити.
		void
		cancel_all();

		//! Доступ к mutex-у, который будет использоваться для внутренней
		//! синхронизации.
		ACE_SYNCH_RECURSIVE_MUTEX &
		mutex();

		//! Тип таймерной нити из ACE.
		typedef ACE_Thread_Timer_Queue_Adapter< ACE_Timer_List >
			adapter_t;

		//! Таймерная нить и очередь из ACE.
		/*!
			Данный объект должен быть уничтожен
			до объекта m_event_handler, т.к. в его деструкторе
			возможны обращения к m_event_handler.
		 */
		std::unique_ptr< adapter_t > m_timer_queue;

		//! Единственный обработчик таймерных событий, который будет
		//! использоваться для всех событий.
		std::unique_ptr< timer_event_handler_t > m_event_handler;

		//! Тип карты для таймерных событий и их идентификаторов
		//! внутри adapter_t.
		typedef std::map<
				timer_act_t*,
				timer_keys_t >
			scheduled_act_to_id_map_t;

		//! Запланированные сообщения
		scheduled_act_to_id_map_t m_scheduled_act_to_id;

		//! Счетчик для собственных идентификаторов таймерных заявок.
		/*!
			Используется простейший механизм для создания идентификаторов:
			последовательно возрастающие значения 64-х битного счетчика.
			Предполагается, что 64-х битного счетчика хватит для генерации
			уникальных идентификаторов даже с большим темпом, даже в течении
			длительного непрерывного времени работы программы.
		 */
		timer_id_t m_self_id_counter;

		//! Тип карты timer_id => идентификатор внутри adapter_t.
		typedef std::map<
				timer_id_t,
				long >
			timer_id_to_ace_id_map_t;

		//! Сопоставление timer_id с идентификаторами,
		//! которые назначает ace.
		timer_id_to_ace_id_map_t m_timer_id_to_ace_id;
	};

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
