/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс локальная очередь событий агента.
*/

#if !defined( _SO_5__RT__IMPL__LOCAL_EVENT_QUEUE_HPP_ )
#define _SO_5__RT__IMPL__LOCAL_EVENT_QUEUE_HPP_

#include <deque>
#include <memory>

#include <ace/Thread_Mutex.h>

#include <so_5/util/h/mutex_pool.hpp>

#include <so_5/rt/h/event_caller_block_ref.hpp>
#include <so_5/rt/h/message_ref.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// event_item_t
//

//! Элемент очереди событий.
struct event_item_t
{
	event_item_t()
	{}

	event_item_t(
		//! Вызыватель события.
		const event_caller_block_ref_t & event_caller_block,
		//! Сообщение.
		const message_ref_t & message_ref )
		:
			m_event_caller_block( event_caller_block ),
			m_message_ref( message_ref )
	{}

	//! Вызыватель события.
	event_caller_block_ref_t m_event_caller_block;

	//! Сообщение.
	message_ref_t m_message_ref;
};

//
// local_event_queue_t
//

//! Локальная очередь событий агента.
/*!
	\note Вся работа с очередью происходит внутри агента.
	Все операции над классом не защищены мутексом.
	Агент владеющий очередью, должен заботиться о защите
	своей очереди сам.
*/
class local_event_queue_t
{
	public:
		explicit local_event_queue_t(
			util::mutex_pool_t< ACE_Thread_Mutex > & mutex_pool );
		~local_event_queue_t();

		//! Получить первую заявку.
		inline void
		pop( event_item_t & event_item );

		//! Вставить заявку в конец.
		inline void
		push(
			const event_item_t & evt );

		//! Получить замок.
		inline ACE_Thread_Mutex &
		lock();

		//! Количество элементов очереди.
		//! \note Не является thread-safe.
		inline size_t
		size() const;

		//! Отчистить очередь.
		inline void
		clear();

	private:
		//! Пул мутексов, от которого получен m_lock.
		/*!
			При создании объекта надо запомнить пул,
			из которого выделяли мутекс, потому что
			при уничтожении объекта надо вернуть мутекс в пул.
		*/
		util::mutex_pool_t< ACE_Thread_Mutex > & m_mutex_pool;

		//! Замок на проведение операций с очередью.
		ACE_Thread_Mutex & m_lock;

		//! Тип для контейнера очереди событий.
		typedef std::deque< event_item_t > events_queue_t;

		//! Контейнер для очереди событий.
		events_queue_t m_events_queue;
};

inline void
local_event_queue_t::pop(
	event_item_t & event_item )
{
	event_item = m_events_queue.front();
	m_events_queue.pop_front();
}

inline void
local_event_queue_t::push(
	const event_item_t & evt )
{
	m_events_queue.push_back( evt );
}

inline ACE_Thread_Mutex &
local_event_queue_t::lock()
{
	return m_lock;
}

inline size_t
local_event_queue_t::size() const
{
	return m_events_queue.size();
}

inline void
local_event_queue_t::clear()
{
	m_events_queue.clear();
}

//! Псевдоним unique_ptr для local_event_queue_t.
typedef std::unique_ptr< local_event_queue_t >
	local_event_queue_unique_ptr_t;

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
