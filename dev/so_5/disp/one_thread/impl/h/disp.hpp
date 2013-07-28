/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реальный класс диспетчера с одной рабочей нитью.
*/

#if !defined( _SO_5__DISP__ONE_THREAD__IMPL__DISP_HPP_ )
#define _SO_5__DISP__ONE_THREAD__IMPL__DISP_HPP_

#include <so_5/rt/h/disp.hpp>
#include <so_5/disp/reuse/work_thread/h/work_thread.hpp>

namespace so_5
{

namespace disp
{

namespace one_thread
{

namespace impl
{

//
// dispatcher_t
//

/*!
	\brief Диспетчер с одной рабочей нитью и очередью заявок.
*/
class dispatcher_t
	:
		public so_5::rt::dispatcher_t
{
	public:
		dispatcher_t();
		virtual ~dispatcher_t();

		//! Реализация методов интерфейса so_5::rt::dispatcher_t.
		//! \{

		//! Запустить диспетчер.
		virtual ret_code_t
		start();

		//! Дать сигнал диспетчеру завершить работу.
		virtual void
		shutdown();

		//! Ожидать полного завершения работы диспетчера
		virtual void
		wait();

		//! Поставить запрос на выполнение события агентом.
		//! Т.е. запланировать вызов события агента.
		virtual void
		put_event_execution_request(
			//! Агент событие которого надо запланировать.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Количество событий,
			//! которые должны произайти у этого агента.
			unsigned int event_count );
		//! \}

	private:
		//! Рабочая нить для диспетчера.
		so_5::disp::reuse::work_thread::work_thread_t m_work_thread;
};


} /* namespace impl */

} /* namespace one_thread */

} /* namespace disp */

} /* namespace so_5 */

#endif
