	/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реальный класс диспетчера активных групп.
*/

#if !defined( _SO_5__DISP__ACTIVE_GROUP__IMPL__DISP_HPP_ )
#define _SO_5__DISP__ACTIVE_GROUP__IMPL__DISP_HPP_

#include <map>

#include <ace/Thread_Mutex.h>

#include <so_5/rt/h/disp.hpp>

namespace so_5 {

namespace rt
{

class agent_t;

} /* namespace rt */


namespace disp {

namespace active_group {

namespace impl {

//
// dispatcher_t
//

/*!
	\brief Диспетчер активных групп.
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
			const so_5::rt::agent_ref_t &,
			unsigned int);
		//! \}

		//! Получить диспетчер активной группы \@ group_name .
		/*! Если диспетчера для такой активной группы нет,
			то он создается делается пометка, что на ней будет вращаться
			1 агент. Если получается, что диспетчер существует, то
			делается пометка, что им будет пользоваться еще 1 агент.
		*/
		so_5::rt::dispatcher_t &
		query_disp_for_group( const std::string & group_name );

		//! Отпустить диспетчер для активной группы \@ group_name .
		/*! Помечает, что на 1 агент меньше теперь пользуется
			этой активной группой. И, если это оказывается последний
			агент этой активной группы, то диспетчер для
			этой активной группы уничтожается.
		*/
		void
		release_disp_for_group( const std::string & group_name );

	private:
		//! Вспомогательный класс для подсчета агентов
		//! работающих в активной группе.
		struct disp_with_ref_t
		{
			disp_with_ref_t()
				:
					m_user_agent( 0 )
			{}

			disp_with_ref_t(
				const so_5::rt::dispatcher_ref_t & disp_ref,
				unsigned int user_agent )
				:
					m_disp_ref( disp_ref ),
					m_user_agent( user_agent)
			{}

			so_5::rt::dispatcher_ref_t m_disp_ref;
			unsigned int m_user_agent;
		};

		//! Тип мапа активных групп к их
		//! диспетчерам с одной рабочей нитью.
		typedef std::map<
				std::string,
				disp_with_ref_t >
			active_group_disp_map_t;

		//! Карта диспетчеров созданных под активные группы.
		active_group_disp_map_t m_group_disp;

		//! Флаг, говорящий о том, что начато завершение
		//! работы диспетчера.
		bool m_shutdown_started;

		//! Замок для операций создания и удаления диспетчеров.
		ACE_Thread_Mutex m_lock;
};

} /* namespace impl */

} /* namespace active_group */

} /* namespace disp */

} /* namespace so_5 */

#endif
