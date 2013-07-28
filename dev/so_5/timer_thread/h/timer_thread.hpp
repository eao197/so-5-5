/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс timer_thread_t.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_THREAD_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/timer_thread/h/timer_act.hpp>
#include <so_5/timer_thread/h/timer_id.hpp>

namespace so_5
{

namespace rt
{

class mbox_ref_t;
class message_ref_t;

} /* namespace rt */

namespace timer_thread
{

//
// timer_thread_t
//

//! Интерфейс нити таймера.
/*!
	Диспетчеру run-time SObjectizer-а необходимы средства
	для работы с отложенными и переодическими сообщениями.
	Точность отсчета времени для различных задач может
	сильно различаться. Для некоторых задач необходима
	точность до милли- или микросекунд,
	для некоторых достаточно точности в секундах.
	Механизмы диспетчеризации для отсчета времени практически
	не играют роли.

	Все реальные таймеры для SObjectizer-а должны быть
	производными от timer_thread_t.

	Настоящий таймер не обязательно должен быть реализован
	в виде нити. Название timer_thread_t сложилось исторически.
	Важно, что диспетчер запускает таймер при своем старте
	посредством метода timer_thread_t::start() и останавливает
	таймер при завершении работы посредством
	timer_thread_t::shutdown(). Т.к. при реализации таймера
	в виде отдельной нити сложно обеспечить, чтобы выход из
	shutdown() означал завершение работы нити таймера, то
	введен метод wait(). Диспетчер вызывает wait() сразу
	после вызова shutdown() и возврат из wait() означает, что
	таймер полностью остановлен и все ресурсы, захваченные
	таймером, освобождены.

	Когда у диспетчера вызывается метод push_delayed_msg
	диспетчер передает сообщение таймеру. Далее таймер отвечает
	за хранение экземпляра сообщения и за определение времени
	когда сообщение должно быть диспетчеризировано.
*/
class SO_5_TYPE timer_thread_t
{
	public:

		timer_thread_t();
		virtual ~timer_thread_t();

		//! Запустить нить таймера.
		virtual ret_code_t
		start() = 0;

		//! Дать сигнал нити таймера завершить работу.
		virtual void
		shutdown() = 0;

		//! Ожидать полного завершения работы нити таймера.
		virtual void
		wait() = 0;

		//! Поставить отложенное или переодическое
		//! сообщение в очередь.
		virtual timer_id_t
		schedule_act(
			timer_act_unique_ptr_t & timer_act ) = 0;

		//! Отменить периодическое сообщение.
		virtual void
		cancel_act(
			timer_id_t msg_id ) = 0;
};

//! Псевдоним unique_ptr для timer_thread_t.
typedef std::unique_ptr< timer_thread_t > timer_thread_unique_ptr_t;

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
