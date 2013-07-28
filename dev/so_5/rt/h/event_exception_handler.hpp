/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс обработчика исключений, которые могут выбрасываться
	в ходе выполнения событий.
*/

#if !defined( _SO_5__RT__EVENT_EXCEPTION_HANDLER_HPP_ )
#define _SO_5__RT__EVENT_EXCEPTION_HANDLER_HPP_

#include <memory>
#include <map>
#include <string>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/agent_ref.hpp>

namespace so_5
{

namespace rt
{

//
// event_exception_response_action_t
//

/*!
	\brief Базовый класс для реакции на возникновение
	исключения в событии агента.

	Объекты наследники данного класса возвращаются
	event_exception_handler_t при обработке исключений.
*/
class SO_5_TYPE event_exception_response_action_t
{
	public:
		virtual ~event_exception_response_action_t();

		//! Выполнить действие-реакцию на возникшее исключение.
		virtual void
		respond_to_exception() = 0;
};

//! Псевдоним на умный указатель %event_exception_response_action_t.
typedef std::unique_ptr< event_exception_response_action_t >
	event_exception_response_action_unique_ptr_t;

//
// event_exception_handler_t
//

class event_exception_handler_t;

//! Псевдоним на умный указатель к event_exception_handler_t.
typedef std::unique_ptr< event_exception_handler_t >
	event_exception_handler_unique_ptr_t;

class so_environment_t;

//
// dispatcher_t
//

//! Интерфейс объекта, который обрабатывает исключение.
class SO_5_TYPE event_exception_handler_t
{
	public:
		virtual ~event_exception_handler_t();

		/*!
			\brief Метод обработки исключения.
			Обрабатывает исключение и возвращает объект,
			который выполняет ответную реакцию на исключение.
		*/
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Среда SObjectizer.
			so_environment_t & so_environment,
			//! Ссылка на экземпляр возникшего сообщения.
			const std::exception & event_exception,
			//! Имя кооперации которой принадлежит агент.
			const std::string & coop_name ) = 0;

		//! Обработка инсталляции.
		/*!
			Если текущий обработчик ставиться в то время, когда
			уже есть обработчик, то новый обработчик должен решить,
			что делать со старым.
			Реализация по умолчанию удаляет старый обработчик.
		*/
		virtual void
		on_install(
			//! Предыдущий обработчик исключений.
			event_exception_handler_unique_ptr_t && previous_handler );
};

//
// create_std_event_exception_handler
//

//! Создать обработчик исключений по умолчанию.
CPP_UTIL_2_EXPORT_FUNC_SPEC( event_exception_handler_unique_ptr_t )
create_std_event_exception_handler();

} /* namespace rt */

} /* namespace so_5 */

#endif
