/*
	SObjectizer 5.
*/

#include <iostream>

#include <so_5/rt/h/event_exception_logger.hpp>

namespace so_5
{

namespace rt
{

//
// event_exception_logger_t
//

event_exception_logger_t::~event_exception_logger_t()
{}

void
event_exception_logger_t::on_install(
	event_exception_logger_unique_ptr_t && previous_logger )
{
	// Реализация по умолчанию удаляет старый журнализатор.
	previous_logger.reset();
}

namespace /* ananymous */
{

//
// std_event_exception_logger_t
//

//! Стандартный логер исключений.
class std_event_exception_logger_t
	:
		public event_exception_logger_t
{
	public:
		virtual ~std_event_exception_logger_t()
		{}

		virtual void
		log_exception(
			//! Ссылка на экземпляр возникшего исключения.
			const std::exception & event_exception,
			//! Имя кооперации, которой принадлежит агент.
			const std::string & coop_name )
		{
			std::cerr
				<< "Sobjectizer event exception cought: "
				<< event_exception.what()
				<< std::endl;
		}
};

} /* ananymous namespace */

//
// create_std_event_exception_logger
//

CPP_UTIL_2_EXPORT_FUNC_SPEC( event_exception_logger_unique_ptr_t )
create_std_event_exception_logger()
{
	return event_exception_logger_unique_ptr_t(
		new std_event_exception_logger_t );
}

} /* namespace rt */

} /* namespace so_5 */
