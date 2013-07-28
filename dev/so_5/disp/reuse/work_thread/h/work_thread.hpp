/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Рабочая нить.
*/

#if !defined( _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_ )
#define _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_

#include <deque>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Atomic_Op.h>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/agent_ref.hpp>

namespace so_5
{

namespace disp
{

namespace reuse
{

namespace work_thread
{

//
// demand_t
//

//! Тип элемента списка заявок.
struct demand_t
{
	//! Агент события которого надо исполнить.
	so_5::rt::agent_ref_t m_agent_ref;

	//! Количество событий которые надо исполнить.
	unsigned int m_event_cnt;

	demand_t()
		:
			m_event_cnt( 0 )
	{}

	demand_t(
		//! Агент у которого надо выполнить события.
		const so_5::rt::agent_ref_t & agent_ref,
		//! Количество выполняемых для агента событий.
		unsigned int event_cnt )
		:
			m_agent_ref( agent_ref ),
			m_event_cnt( event_cnt )
	{}
};

//! Тип для контейнера очереди заявок на выполнение событий.
typedef std::deque< demand_t > demand_container_t;

//
// demand_queue_t
//

//! Очередь заявок.
/*!
	Так же хранит признак необходимости завершения
	работы.

	Предназначена для использования несколькими
	нитями одновременно.
*/
class demand_queue_t
{
	public:
		demand_queue_t();
		~demand_queue_t();

		//! Поместить заявку на исполнение событий в очередь.
		void
		push(
			//! Агент у которого надо выполнить события.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Количество выполняемых для агента событий.
			unsigned int event_cnt );

		enum
		{
			//! Была извлечена заявка.
			demand_extracted = 1,
			//! Заявка не была извлечена, т.к.
			//! выставлен признак завершения работы.
			shutting_down = 2,
			//! Заявка не была извлечена, т.к. в очереди нет заявок.
			no_demands = 3
		};

		//! Взять имеющиеся заявки.
		/*!
			Если заявок в очереди нет, то текущая нить
			засыпает до появления заявок в очереди, либо
			до выставления признака завершения работы.
		*/
		int
		pop(
			/*! Приемник зaявок. */
			demand_container_t & queue_item );

		//! Начать обслуживание заявок.
		void
		start_service();

		//! Остановить обслуживание заявок.
		void
		stop_service();

		//! Отчистить очередь.
		void
		clear();

	private:
		//! Контейнер очереди.
		demand_container_t m_demands;

		//! Синхронизация.
		//! \{
		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_not_empty;
		//! \}

		//! Флаг - обслуживать ли клиентов очереди?.
		/*! Принимает значения:
			true - надо продолжать работу - обслуживать методы push/pop.
			false - прекратить обслуживание.
		*/
		bool m_in_service;
};

//
// work_thread_t
//

//!	Класс рабочей нити.
/*!
	Рабочая нить должна использоваться в составе какого-либо
	диспетчера. При этом время жизни объекта-диспетчера должно
	превышать время жизни объекта-нити.
*/
class work_thread_t
{
	public:
		work_thread_t(
			rt::dispatcher_t & disp );

		~work_thread_t();

		//! Поставить запрос на выполнение события агентом.
		//! Т.е. запланировать вызов события агента.
		void
		put_event_execution_request(
			//! Агент событие которого надо запланировать.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Количество событий,
			//! которые должны произайти у этого агента.
			unsigned int event_count );

		//! Запустить нить.
		void
		start();

		//! Дать сигнал к останову работы.
		void
		shutdown();

		//! Ожидать завершения работы.
		/*!
			После останова нити удаляются все,
			оставшиеся не обработанными, заявки.
		*/
		void
		wait();

	protected:
		//! Основное тело циклической работы.
		void
		body();

		//! Обработка исключения.
		void
		handle_exception(
			//! Отловленное исключение выброшенное из обрабтчика
			//! какого-либо события.
			const std::exception & ex,
			//! Агент чей обработчик выбросил исключение.
			const so_5::rt::agent_ref_t & a_exception_producer );

		//! Обслужить блок заявок.
		void
		serve_demands_block(
			//! Список заявок для обслуживания.
			demand_container_t & executed_demands );

		//! Точка входа в нить для ACE_Thread_Manager.
		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

	private:
		//! Очередь заявок для данной рабочей нити.
		demand_queue_t m_queue;

		//! Тип флага - продолжать выполнять работу?
		enum
		{
			//! 0 - нет, не продолжать.
			WORK_THREAD_STOP = 0,
			//! 1 - да, продолжать.
			WORK_THREAD_CONTINUE = 1
		};

		//! Флаг - продолжать выполнять работу?
		/*! Может принимать значения:
			WORK_THREAD_STOP, WORK_THREAD_CONTINUE
		*/
		ACE_Atomic_Op< ACE_Thread_Mutex, long > m_continue_work;

		//! Идентификатор нити, созданной для данного объекта.
		/*!
			\note Значение актуально только после вызова start().
		*/
		ACE_thread_t m_tid;

		//! Диспетчер запустивший рабочую нить.
		/*!
			При обработке отловленного исключения, от диспетчера
			берется объект, которому делегируется обработка.
		*/
		rt::dispatcher_t & m_disp;
};

} /* namespace work_thread */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

#endif
