/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Публичный интерфейс средств управления реакторами.
*/

#if !defined( _SO_5_TRANSPORT__ACE__REACTORS_HPP_ )
#define _SO_5_TRANSPORT__ACE__REACTORS_HPP_

#include <memory>
#include <string>
#include <map>

#include <ace/Reactor.h>
#include <ace/Task.h>

#include <so_5/h/log_err.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/ret_code.hpp>

#include <so_5/h/types.hpp>

namespace so_5_transport
{

namespace ace
{

//
// reactor_instance_t
//

//! Интерфейс, за которым спрятан реактор и необходимые для него ресурсы.
class SO_5_TRANSPORT_TYPE reactor_instance_t
{
	public :
		virtual ~reactor_instance_t();

		//! Запуск реактора.
		virtual so_5::ret_code_t
		start() = 0;

		//! Инициировать остановку реактора.
		/*!
			Возврат из данного метода должен происходить
			только после полного останова всех нитей, которые
			были задействованны для данного реактора.
		*/
		virtual so_5::ret_code_t
		stop() = 0;

		//! Дождаться останова всех нитей реактора.
		virtual so_5::ret_code_t
		wait() = 0;

		//! Реактор.
		/*!
			\return Указатель на реактор, который скрывается за
			данным интерфейсом.
		*/
		virtual ACE_Reactor *
		reactor() = 0;
};

//
// reactor_instance_unique_ptr_t
//

//! Псевдоним unique_ptr для reactor_instance.
typedef std::unique_ptr< reactor_instance_t >
	reactor_instance_unique_ptr_t;

//! Тип умного указателя на экземпляр reactor_instance.
typedef std::shared_ptr< reactor_instance_t >
	reactor_instance_ref_t;

//! Тип карты реакторов.
typedef std::map< std::string, reactor_instance_ref_t >
	reactor_instance_map_t;

/*!
	\name Функции создания конкретных типов реакторов.
	\{
*/

//
// make_select_reactor
//

//! Создание реактора, основанного на ACE_Select_Reactor.
SO_5_TRANSPORT_EXPORT_FUNC_SPEC( reactor_instance_unique_ptr_t )
make_select_reactor();

//
// make_tp_reactor
//

//! Создание реактора, основанного на ACE_TP_Reactor.
SO_5_TRANSPORT_EXPORT_FUNC_SPEC( reactor_instance_unique_ptr_t )
make_tp_reactor(
	//! Количество нитей, на которых должна вестись работа.
	size_t thread_count );

/*!
	\}
*/

/*!
	\name Средства для создания собственных оберток вокруг реакторов.
	\{
*/

//
// end_reactor_event_loop_call_handler_t
//

//! Класс для завершения работы реактора на его нити.
class SO_5_TRANSPORT_TYPE end_reactor_event_loop_call_handler_t
	:
		public ACE_Event_Handler
{
	public:
		end_reactor_event_loop_call_handler_t( ACE_Reactor * r );

		//! Вызывает у реактора end_reactor_event_loop().
		virtual int
		handle_exception( ACE_HANDLE );

		virtual int
		handle_close( ACE_HANDLE , ACE_Reactor_Mask );

	protected:
		//! Чтобы удаление обработчика происходило только из handle_close().
		virtual ~end_reactor_event_loop_call_handler_t();
};

//
// reactor_task_template_t
//

/*!
	\brief Шаблон для ACE_Task, который способен работать с разными
	типами реакторов и разным количеством нитей.

	\tparam REACTOR_TYPE тип реактора, который должен использоваться.
*/
template< class REACTOR_TYPE >
class reactor_task_t
	:
		public ACE_Task< ACE_MT_SYNCH >
{
		typedef ACE_Task< ACE_MT_SYNCH > base_type_t;
	public:
		//! Инициализирующий конструктор.
		reactor_task_t(
			//! Количество нитей, на которых должен работать реактор.
			size_t thread_count )
			:
				m_thread_count( thread_count ),
				m_active_threads( 0 )
			{}

		virtual ~reactor_task_t()
		{}

		//! Запустить задачу.
		/*!
			Запускаются необходимые нити.
		*/
		virtual so_5::ret_code_t
		start()
		{
			ACE_TRACE( "so_5::ace::reactor_task_t::start()" );

			m_reactor.reset( new ACE_Reactor( new REACTOR_TYPE, true ) );
			m_active_threads = m_thread_count;

			if( -1 == activate(
				THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,
				m_thread_count ) )
			{
				ACE_ERROR( (
					LM_ERROR,
					SO_5_LOG_FMT( "unable to activate reactor task" ) ) );

				m_active_threads = 0;
				m_reactor.reset();
				return rc_reactor_task_activation_failed;
			}

			return 0;
		}

		//! Инициировать остановку задачи.
		virtual void
		stop()
		{
			ACE_TRACE( "so_5::ace::reactor_task_t::stop()" );

			// Ставим end_reactor_event_loop_call_handler_t
			// на обработку реатором, а он в свою очередь инициирует
			// остановку event_loop с нити ректора.

			end_reactor_event_loop_call_handler_t * end_loop_handler =
				new end_reactor_event_loop_call_handler_t(
					m_reactor.get() );

			if( !end_loop_handler )
			{
				ACE_ERROR( (
					LM_ERROR,
					SO_5_LOG_FMT( "unable to create "
						"end_reactor_event_loop_call_handler_t" ) ) );

				ACE_OS::abort();
			}

			m_reactor->notify( end_loop_handler );
		}

		//! Выполнение основного цикла выборки сообщений реактора.
		virtual int
		svc()
		{
			ACE_TRACE( "so_5::ace::reactor_task_t::svc()" );

			m_reactor->owner( ACE_Thread::self() );

			// Выполняем цикл.
			m_reactor->run_reactor_event_loop();

			// Если этот поток последний из потоков реактора.
			// То завершаем работу.
			if( 0 == --m_active_threads )
			{
				m_reactor->purge_pending_notifications( 0 );
				m_reactor.reset();
			}

			return 0;
		}

		//! Получить доступ к реактору.
		virtual ACE_Reactor *
		reactor()
		{
			return m_reactor.get();
		}

	private:
		//! Счетчик потоков не которых работает реактор.
		/*!
			Последний работающий поток, должен будет уничтожить реактор.
		*/
		so_5::atomic_counter_t m_active_threads;

		//! Реактор, который будет использоваться.
		std::unique_ptr< ACE_Reactor > m_reactor;

		//! Количество нитей, на которых будет работать реактор.
		const size_t m_thread_count;
};

//
// reactor_instance_template_t
//

/*!
	\brief Реализация интерфейса reactor_instance на основе шаблона.

	\param REACTOR_TYPE тип реактора, который должен использоваться.
 */
template< class REACTOR_TYPE >
class reactor_instance_template_t
	:
		public reactor_instance_t
{
	public :
		//! Инициализирующий конструктор.
		reactor_instance_template_t(
			//! Количество нитей, на которых должен работать реактор.
			size_t thread_count )
			:
				m_reactor_task( thread_count )
		{}

		virtual ~reactor_instance_template_t()
		{}

		/*!
			\name Реализация унаследованных методов.
			\{
		*/
		virtual so_5::ret_code_t
		start()
		{
			return m_reactor_task.start();
		}

		virtual so_5::ret_code_t
		stop()
		{
			m_reactor_task.stop();
			return 0;
		}

		virtual so_5::ret_code_t
		wait()
		{
			m_reactor_task.wait();
			return 0;
		}

		virtual ACE_Reactor *
		reactor()
		{
			return m_reactor_task.reactor();
		}
		/*!
			\}
		*/

	private :
		//! Реализация реактора и необходимых нитей.
		reactor_task_t< REACTOR_TYPE > m_reactor_task;
};

/*!
	\}
*/

} /* namespace ace */

} /* namespace so_5_transport */

#endif

