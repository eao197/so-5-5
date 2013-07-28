/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс диспетчера - dispatcher_t.
*/

#if !defined( _SO_5__RT__DISP_HPP_ )
#define _SO_5__RT__DISP_HPP_

#include <memory>
#include <map>
#include <string>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

namespace so_5
{

namespace rt
{

//
// disp_evt_except_handler_t
//

//! Обработчик исключений событий агентов на данном диспетчере.
class SO_5_TYPE disp_evt_except_handler_t
{
	public:
		virtual ~disp_evt_except_handler_t();

		//! Обработать исключение.
		virtual event_exception_response_action_unique_ptr_t
		handle_exception(
			//! Ссылка на экземпляр возникшего исключения.
			const std::exception & event_exception,
			//! Имя кооперации, которой принадлежит агент.
			const std::string & coop_name ) = 0;
};

//
// dispatcher_t
//

//! Абстрактный базовый класс для всех диспетчеров.
/*!
	Диспетчеры отвечают за вызов событий агентов.

	Когда регистрируется кооперация агентов, каждый агент
	кооперации привязывается через привязыватель dispatcher_binder_t
	к совему диспетчеру.

	Когда агентам через mbox отправляетя сообщения, на которое те подписаны,
	оно обращается к методам агента, которые в свою очередь кладут
	необходисые для выполнения обрабочика данные к себе в локальную очередь,
	а затем обращаются к своему диспетчеру, чтобы тот поставил запрос
	на выполнение заявок и исполнил его, как только сможет.
	Т.е. диспетчерам говорится, что есть необходимость выполнять события,
	а те в свою очередь инициируют их выполнение на рабочей нити (нитях).
*/
class SO_5_TYPE dispatcher_t
{
	public:
		dispatcher_t();
		virtual ~dispatcher_t();

		inline dispatcher_t *
		self_ptr()
		{
			return this;
		}

		//! Запустить диспетчер.
		/*!
			\return 0, если диспетчер успешно запущен,
			и rc_disp_start_failed если запустить диспетчер
			не удалось.
		*/
		virtual ret_code_t
		start() = 0;

		//! Дать сигнал диспетчеру завершить работу.
		/*!
			Должен только устанавливаться признак необходимости
			завершения работы диспетчера. Метод shutdown не
			обязан дожидаться завершения работы.
		*/
		virtual void
		shutdown() = 0;

		//! Ожидать полного завершения работы диспетчера.
		/*!
			Возврат из этого метода должен осуществляться только
			когда работа диспетчера полностью завершена.
		*/
		virtual void
		wait() = 0;

		//! Поставить запрос на выполнение события агентом.
		//! Т.е. запланировать вызов события агента.
		virtual void
		put_event_execution_request(
			//! Агент событие которого надо запланировать.
			const agent_ref_t & agent_ref,
			//! Количество событий,
			//! которые должны произайти у этого агента.
			unsigned int event_count ) = 0;

		//! Установить новый disp_evt_except_handler.
		void
		set_disp_event_exception_handler(
			disp_evt_except_handler_t & disp_evt_except_handler );

		disp_evt_except_handler_t &
		query_disp_evt_except_handler();

	private:
		//! Обработчик исключений событий агентов на данном диспетчере.
		disp_evt_except_handler_t * m_disp_evt_except_handler;
};

//! Тип для умного указателя на dispatcher_t.
typedef std::unique_ptr< dispatcher_t > dispatcher_unique_ptr_t;

//! Тип для умного указателя на dispatcher_t.
typedef std::shared_ptr< dispatcher_t >
	dispatcher_ref_t;

//! Тип карты для хранения именованных диспетчеров.
typedef std::map<
		std::string,
		dispatcher_ref_t >
	named_dispatcher_map_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
