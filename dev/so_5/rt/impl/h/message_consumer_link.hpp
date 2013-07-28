/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейсный класс для звена цепи потребителей сообщения.

*/

#if !defined( _SO_5__RT__IMPL__MESSAGE_CONSUMER_LINK_HPP_ )
#define _SO_5__RT__IMPL__MESSAGE_CONSUMER_LINK_HPP_

#include <memory>

#include <so_5/h/types.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/event_handler_caller_ref.hpp>
#include <so_5/rt/h/event_caller_block_ref.hpp>

namespace so_5
{

namespace rt
{

// Сообщаем, что есть такой класс.
class message_ref_t;

namespace impl
{

class message_consumer_link_t;

//! Класс unique_ptr для message_consumer_link_t.
typedef std::unique_ptr< message_consumer_link_t >
	message_consumer_link_unique_ptr_t;

//! Умный указатель с подсчетом ссылок на message_consumer_link_t.
typedef std::shared_ptr< message_consumer_link_t >
	message_consumer_link_ref_t;

//
// message_consumer_link_t
//

//! Интерфейсный класс для звена цепочки потребителей сообщения.
class message_consumer_link_t
{
	public:
		message_consumer_link_t(
			//! Ссылка на агента.
			const agent_ref_t & agent_ref );

		~message_consumer_link_t();

		//! Диспетчеризировать сообщение.
		void
		dispatch( const message_ref_t & message );

		//! Установить левого соседа.
		void
		set_left(
			const message_consumer_link_ref_t & left );

		//! Установить правого соседа.
		void
		set_right(
			const message_consumer_link_ref_t & right );

		//! Получить левого соседа.
		inline const message_consumer_link_ref_t &
		query_left() const
		{
			return m_left;
		}

		//! Получить правого соседа.
		inline const message_consumer_link_ref_t
		query_right() const
		{
			return m_right;
		}

		//! Является ли элемент первым в цепочке.
		inline bool
		is_first()
		{
			return 0 == m_left.get();
		}

		//! Является ли элемент последним в цепочке.
		inline bool
		is_last()
		{
			return 0 == m_right.get();
		}

		inline const event_caller_block_ref_t &
		event_caller_block() const
		{
			return m_event_handler_caller;
		}

		//! Получить вызывателей.
		inline event_caller_block_ref_t &
		event_caller_block()
		{
			return m_event_handler_caller;
		}

	protected:
		//! Левый сосед.
		message_consumer_link_ref_t m_left;

		//! Правый сосед.
		message_consumer_link_ref_t m_right;

		//! Блок вызыватель обработчика.
		event_caller_block_ref_t m_event_handler_caller;

		//! Ссылка на агента, к обработчику которого данная привязка
		//! и существует.
		agent_ref_t m_agent_ref;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
