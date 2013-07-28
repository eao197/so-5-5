/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реализация анонимного mbox-а.
*/

#if !defined( _SO_5__RT__IMPL__LOCAL_MBOX_HPP_ )
#define _SO_5__RT__IMPL__LOCAL_MBOX_HPP_

#include <so_5/h/types.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/mbox_ref.hpp>

#include <so_5/rt/impl/h/mbox_core.hpp>
#include <so_5/rt/impl/h/message_distributor.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// local_mbox_t
//

//! Локальный анонимный mbox.
class local_mbox_t
	:
		public mbox_t
{
		friend class impl::mbox_core_t;

		explicit local_mbox_t(
			impl::mbox_core_t & mbox_core );

		local_mbox_t(
			impl::mbox_core_t & mbox_core,
			ACE_RW_Thread_Mutex & lock );

	public:
		virtual ~local_mbox_t();

		//! Переопределение методов базового класса.
		//! \{

		//! Получить имя.
		virtual const std::string &
		query_name() const;

	protected:
		//! Добавить потребителя сообщения,
		//! который является обработчиком события.
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
				event_handler_caller_ref );

		//! Добавить потребителя сообщения,
		//! который является обработчиком события.
		/*!
			Метод вызывается, когда агент подписывается на
			тип сообщение, на которое он ранее уже подписывался.
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
			throwing_strategy_t throwing_strategy );

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
			throwing_strategy_t throwing_strategy );

		//! Удалить всю подписку агента на заданный тип сообщения.
		//! \note Вызывается при дерегистрации агента,
		//! когда надо удалять все его подписки,
		//! а не отдельные вызыватели.
		virtual void
		unsubscribe_event_handler(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t *
				message_consumer_link );
		//! \}

		//! Отправить всем подписчикам сообщение.
		void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

	private:
		//! Создатель данного mbox-а.
		impl::mbox_core_ref_t m_mbox_core;

		//! Ссылка на мутекс для синхронизации операций подписки.
		ACE_RW_Thread_Mutex & m_lock;

		//! Распределитель сообщений.
		impl::message_distributor_t m_message_distributor;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
