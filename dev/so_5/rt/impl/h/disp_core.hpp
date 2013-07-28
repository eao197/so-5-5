/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс для обеспечения работы с диспетчерами.
*/

#if !defined( _SO_5__RT__IMPL__DISP_CORE_HPP_ )
#define _SO_5__RT__IMPL__DISP_CORE_HPP_

#include <ace/Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/rt/h/disp.hpp>

#include <so_5/rt/h/event_exception_logger.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// disp_core_t
//

//! Класс для обеспечения работы с диспетчерами.
class disp_core_t
	:
		public disp_evt_except_handler_t
{
	public:
		disp_core_t(
			so_environment_t & so_environment,
			const named_dispatcher_map_t & named_dispatcher_map,
			event_exception_logger_unique_ptr_t && logger,
			event_exception_handler_unique_ptr_t && handler );

		virtual ~disp_core_t();

		//! Получить диспетчер по умолчанию.
		dispatcher_t &
		query_default_dispatcher();

		//! Получить именованный диспетчер.
		/*!
			\return Если диспетчер с таким именем есть,
			то вернется dispatcher_ref_t, который указывает на него,
			в противном случае вернется dispatcher_ref_t на 0.
		*/
		dispatcher_ref_t
		query_named_dispatcher(
			const std::string & disp_name );

		//! Запустить работу всех диспетчеров.
		ret_code_t
		start();

		//! Сообщить всем диспетчерам, что надо завершать работу.
		void
		shutdown();

		//! Ждать завершения всех диспетчеров.
		void
		wait();

		//! Установить другой логер исключений.
		void
		install_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		//! Установить другой обработчик исключений.
		void
		install_exception_handler(
			event_exception_handler_unique_ptr_t && handler );

		//! Обработать исключение.
		/*!
			Логирует исключение и создает его обработчик.
		*/
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Ссылка на экземпляр возникшего исключения.
			const std::exception & event_exception,
			//! Имя кооперации, которой принадлежит агент.
			const std::string & coop_name );

	private:
		//! Среда SObjectizer.
		so_environment_t & m_so_environment;

		//!Диспетчер по умолчанию.
		dispatcher_unique_ptr_t m_default_dispatcher;

		//! Именованные диспетчеры.
		named_dispatcher_map_t m_named_dispatcher_map;

		//! Замок для замещения логера исключений.
		ACE_Thread_Mutex m_exception_logger_lock;

		//! Логер исключений.
		event_exception_logger_unique_ptr_t m_event_exception_logger;

		//! Обработчик исключений.
		event_exception_handler_unique_ptr_t m_event_exception_handler;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
