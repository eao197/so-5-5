/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс, который распределяет сообщения их потребителям.
*/

#if !defined( _SO_5__RT__IMPL__MESSAGE_DISTRIBUTOR_HPP_ )
#define _SO_5__RT__IMPL__MESSAGE_DISTRIBUTOR_HPP_

#include <so_5/rt/impl/h/message_consumer_chain.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

/*!
	Класс, который хранит инфраструктуру
	<b>тип сообщения => потребитель сообщения</b>.
	Добавляет новых потребителей и в случае вырождения
	какой-либо цепочки для определенного типа сообщений
	удаляет ее.
*/
class message_distributor_t
{
	public:
		message_distributor_t();
		virtual ~message_distributor_t();

		//! Добавить потребителя сообщения в конец цепочки.
		/*!
			Через этот метод добавлябются потребители сообщения,
			которые несут за собой привязку к обработчику событий
			и должны вставать в конец.
		*/
		void
		push_first(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения, который
			//! будет вставлен.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! Первый вызыватель, который будет вставлен.
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		ret_code_t
		push_more(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения, который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t * message_consumer_link,
			//! Очередной вызыватель для подписки.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Бросать ли исключения в случае подписки.
			throwing_strategy_t throwing_strategy );

		//! Добавить потребителя сообщения в конец цепочки.
		/*!
			Через этот метод добавлябются потребители сообщения,
			которые несут за собой привязку к обработчику событий
			и должны вставать в конец.
		*/
		ret_code_t
		pop(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения, который содержит
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

		//! Удалить всю подписку данного агента на данный
		//! тип сообщений.
		void
		pop(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Указатель на потребителя сообщения, который содержит
			//! ранее созданные подписки.
			impl::message_consumer_link_t *
				message_consumer_link );

		//! Разослать сообщение потребителям.
		void
		distribute_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

	private:
		/*!
			Получить ссылку на цепочку потребителей
			заданного типа сообщений.
			Если цепочки для данного типа сообщений
			еще не существует, то она будет создана и добавлена.
		*/
		message_consumer_chain_t &
		provide_message_consumer_chain(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper );

		//! Карта тип сообщения => цепочка потребителей.
		msg_type_to_consumer_chain_map_t
			m_msg_type_to_consumer_chain_map;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
