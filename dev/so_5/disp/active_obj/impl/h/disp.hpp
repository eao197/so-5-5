/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реальный класс диспетчера акитвных объектов.
*/

#if !defined( _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_HPP_ )
#define _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_HPP_

#include <map>

#include <ace/Thread_Mutex.h>

#include <so_5/rt/h/disp.hpp>

namespace so_5 {

namespace rt
{
	class agent_t;

} /* namespace rt */


namespace disp
{

namespace active_obj
{

namespace impl
{

//
// dispatcher_t
//

/*!
	\brief Диспетчер активных объектов.
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
		/*! \note Не должен вызываться напрямую у этого диспетчера. */
		virtual void
		put_event_execution_request(
			//! Агент событие которого надо запланировать.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Количество событий,
			//! которые должны произайти у этого агента.
			unsigned int event_count );
		//! \}

		so_5::rt::dispatcher_t &
		create_disp_for_agent( const so_5::rt::agent_t & agent );

		void
		destroy_disp_for_agent( const so_5::rt::agent_t & agent );

	private:
		typedef std::map<
				const so_5::rt::agent_t *,
				so_5::rt::dispatcher_ref_t >
			agent_disp_map_t;

		//! Карта диспетчеров созданных под агентов.
		agent_disp_map_t m_agent_disp;

		//! Флаг, говорящий о том, что начато завершение
		//! работы диспетчера.
		bool m_shutdown_started;

		//! Замок для операций создания и удаления диспетчеров.
		ACE_Thread_Mutex m_lock;
};

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */

#endif
