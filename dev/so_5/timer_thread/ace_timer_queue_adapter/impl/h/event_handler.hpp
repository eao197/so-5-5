/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Обработчик таймерных событий.
 */

#if !defined( _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__EVENT_HANDLER_HPP_ )
#define _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__EVENT_HANDLER_HPP_

#include <ace/Event_Handler.h>

#include <so_5/timer_thread/h/timer_act.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

namespace impl
{

//
// timer_act_utilizator_t
//

//! Утилизатор таймерных действий.
class timer_act_utilizator_t
{
	public:
		timer_act_utilizator_t(){}
		virtual ~timer_act_utilizator_t(){}

		//! При необходимости утилизировать таймерное действие.
		virtual void
		utilize_timer_act( timer_act_t * timer_act ) = 0;
};


//
// timer_event_handler_t
//

/*!
	\brief Обработчик таймерных событий.

	Экземпляр этого типа регистрируется, как обработчик всех таймерных
	событий у ACE_Thread_Timer_Queue_Adapter.
*/

class timer_event_handler_t
	:
		public ACE_Event_Handler
{
	public:
		//! Инициализирующий конструктор.
		timer_event_handler_t(
				timer_act_utilizator_t & utilizator );
		virtual ~timer_event_handler_t();

		//! Реакция на наступление таймерного события.
		/*!
			\return 0
		 */
		virtual int
		handle_timeout(
				const ACE_Time_Value &,
				const void * raw_act );

	private:
		//! Объект, отвечающий за утилизацию Timer ACT.
		timer_act_utilizator_t & m_utilizator;
};

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
