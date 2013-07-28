/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс event_exception_logger_t.
*/

#if !defined( _SO_5__RT__EVENT_EXCEPTION_LOGGER_HPP_ )
#define _SO_5__RT__EVENT_EXCEPTION_LOGGER_HPP_

#include <exception>
#include <string>
#include <memory>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

//
// event_exception_logger_t
//

class event_exception_logger_t;

//! Псевдоним на умный указатель к event_exception_logger_t.
typedef std::unique_ptr< event_exception_logger_t >
	event_exception_logger_unique_ptr_t;

//! Интерфейс объекта, который логирует исключение.
class SO_5_TYPE event_exception_logger_t
{
	public:
		virtual ~event_exception_logger_t();

		/*!
			\brief Метод логирования исключения.
			Логирование определяется наследниками класса.
		*/
		virtual void
		log_exception(
			//! Ссылка на экземпляр возникшего исключения.
			const std::exception & event_exception,
			//! Имя кооперации, которой принадлежит агент.
			const std::string & coop_name ) = 0;

		//! Обработка инсталляции.
		/*! Если текущий журнализатор ставиться в то время, когда
			уже есть журнализатор, то новый должен решить,
			что делать со старым.
			Реализация по умолчанию удаляет старый журнализатор.
		*/
		virtual void
		on_install(
			//! Предыдущий журнализатор.
			event_exception_logger_unique_ptr_t && previous_logger );
};

//
// create_std_event_exception_logger
//

//! Создать логер исключений по умолчанию.
CPP_UTIL_2_EXPORT_FUNC_SPEC( event_exception_logger_unique_ptr_t )
create_std_event_exception_logger();

} /* namespace rt */

} /* namespace so_5 */

#endif
