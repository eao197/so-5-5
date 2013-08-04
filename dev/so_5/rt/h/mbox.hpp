/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс mbox_t.

	Базовый класс для mbox-ов.
*/


#if !defined( _SO_5__RT__MBOX_HPP_ )
#define _SO_5__RT__MBOX_HPP_

#include <string>
#include <memory>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/h/declspec.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/message_ref.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_handler_caller_ref.hpp>

namespace so_5
{

namespace timer_thread
{
	class timer_act_t;
}

namespace rt
{

namespace impl
{

class message_consumer_link_t;
class named_local_mbox_t;

} /* namespace impl */

class agent_t;
class agent_ref_t;
class mbox_ref_t;
class message_ref_t;

//
// mbox_t
//

//! Базовый класс почтового ящика.
/*!
	Служит интерфейсом для отправки и получения сообщений агентами.

	Mbox-ы создаются через SObjectizer Environment и
	оборачиваются в so_5::rt::mbox_ref_t.
	Главный метод mbox-а – это метод доставки сообщения
	mbox_t::deliver_message(), который имеет две версии:
	одна предназначена для отправкой реального экземпляра сообщения,
	а вторая для отправки сообщения без реального объекта,
	когда известен только тип сообщения.
	Суть метода сводится к определению списка всех агентов
	подписчиков на данный тип сообщения и к последующему добавлению
	заявки на обработку сообщения в локальную очередь
	событий каждому агенту (agent_t) из этого списка.

	Отправка отложенных и периодических событий происходит тоже
	через mbox_t. SObjectizer Environment запоминает mbox,
	через который осуществляется отправка сообщения, а таймерная нить
	в назначенное время инициирует отправку сообщения через заданный mbox.
	\see so_environment_t::schedule_timer(), so_environment_t::single_timer().
*/
class SO_5_TYPE mbox_t
	:
		private atomic_refcounted_t
{
		friend class agent_t;
		friend class mbox_ref_t;
		friend class impl::named_local_mbox_t;
		friend class so_5::timer_thread::timer_act_t;

		mbox_t( const mbox_t & );
		void
		operator = ( const mbox_t & );

	public:
		mbox_t();
		virtual ~mbox_t();

		//! Отправить сообщение на данный mbox.
		/*!
			Экземпляр сообщения определяется берется из
			\a msg_unique_ptr, далее за удаление сообщения
			отвечает SO.
		*/
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Умный указатель на экземпляр сообщения.
			std::unique_ptr< MESSAGE > & msg_unique_ptr );


		//! Отправить сообщение на данный mbox.
		/*!
			Экземпляр сообщения определяется берется из
			\a msg_unique_ptr, далее за удаление сообщения
			отвечает SO.
		*/
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Умный указатель на экземпляр сообщения.
			std::unique_ptr< MESSAGE > && msg_unique_ptr );


		//! Отправить сообщение на данный mbox.
		/*!
			Для тех случаев когда сообщение является сигналом
			и реального экземпларя сообщения создавать не нужно.
		*/
		template< class MESSAGE >
		inline void
		deliver_message();

		//! Получить имя.
		virtual const std::string &
		query_name() const = 0;

	protected:
		//! Добавить потребителя сообщения,
		//! который является обработчиком события.
		/*!
			Метод вызывается, когда агент первый раз подписывается
			на сообщение заданного типа.
		*/
		virtual void
		subscribe_first_event_handler(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения, который
			//! будет вставлен.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! Первый вызыватель, который будет вставлен.
			const event_handler_caller_ref_t &
				event_handler_caller_ref ) = 0;

		//! Добавить потребителя сообщения,
		//! который является обработчиком события.
		/*!
			Метод вызывается, когда агент подписывается на
			тип сообщения, на которое он ранее уже подписывался.
		*/
		virtual ret_code_t
		subscribe_more_event_handler(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t * message_consumer_link,
			//! Очередной вызыватель для подписки.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Бросать ли исключения в случае подписки.
			throwing_strategy_t throwing_strategy ) = 0;

		//! Удалить потребителя сообщения,
		//! который является обработчиком события.
		virtual ret_code_t
		unsubscribe_event_handler(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t *
				message_consumer_link,
			//! Вызыватель, который надо удалить.
			const event_handler_caller_ref_t &
				event_handler_caller_ref,
			//! Приемник для флага, является ли удаляемый
			//! вызыватель последним в подписке агента на заданный
			//! тип сообщения.
			bool & is_last_subscription,
			//! Бросать ли исключения в случае подписки.
			throwing_strategy_t throwing_strategy ) = 0;

		//! Удалить всех потребителей сообщения.
		virtual void
		unsubscribe_event_handler(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t *
				message_consumer_link ) = 0;

		//! Отправить всем подписчикам сообщение.
		virtual void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref ) = 0;

		//! Адрес для проведения операция сравнения.
		/*!
			Реализация по умолчанию возвращает this.
		*/
		virtual const mbox_t *
		cmp_ordinal() const;
};


template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > & msg_unique_ptr )
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > && msg_unique_ptr )
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_message()
{
	deliver_message(
		type_wrapper_t( typeid( MESSAGE ) ),
		message_ref_t() );
}

} /* namespace rt */

} /* namespace so_5 */

#endif
