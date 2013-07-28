/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Шаблонный класс event_data_t.

	Шаблон, конкретные экземпляры которого служат
	параметрами для методов обработки событий.
*/


#if !defined( _SO_5__RT__EVENT_DATA_HPP_ )
#define _SO_5__RT__EVENT_DATA_HPP_

#include <so_5/rt/h/message.hpp>

#include <so_5/rt/h/mbox_ref.hpp>

namespace so_5
{

namespace rt
{

class mbox_t;

//! Шаблонный класс для инкапсуляции сообщения,
//! которое является параметром обработчика события агента.
/*!
	\code
		void
		a_sample_t::evt_smth(
			const so_5::rt::event_data_t< sample_message_t > & msg )
		{
			// ...
		}
	\endcode
*/
template< class MESSAGE >
class event_data_t
{
	public:
		//! Конструктор по умолчанию, который
		//! создает пустое сообщение - сигнал.
		event_data_t()
			:
				m_message_instance( 0 )
		{}

		event_data_t( const MESSAGE * message_instance )
			:
				m_message_instance( message_instance )
		{}

		~event_data_t()
		{}

		//! Получить ссылку на сообщение.
		const MESSAGE&
		operator * () const
		{
			return *m_message_instance;
		}

		//! Получить указатель на сообщение.
		const MESSAGE *
		get() const
		{
			return m_message_instance;
		}

		//! Вызвать метод у сообщения.
		const MESSAGE *
		operator -> () const
		{
			return get();
		}

	private:
		//! Указатель на экземпляр сообщения.
		const MESSAGE * const m_message_instance;
};

//! Шаблонный класс для инкапсуляции сообщения,
//! которое является параметром обработчика события агента, и которое
//! гарантировано указывает на реально существующий объект сообщения.
/*!
	\code
		void
		a_sample_t::evt_smth(
			const so_5::rt::not_null_event_data_t< sample_message_t > & msg )
		{
			// ...
		}
	\endcode
*/
template< class MESSAGE >
class not_null_event_data_t
{
	public:
		//! Конструктор по умолчанию, который
		//! создает пустое сообщение - сигнал.
		not_null_event_data_t( const MESSAGE & message_instance )
			:
				m_message_instance( message_instance )
		{}

		~not_null_event_data_t()
		{}

		//! Получить ссылку на сообщение.
		const MESSAGE&
		operator * () const
		{
			return *m_message_instance;
		}

		//! Получить указатель на сообщение.
		const MESSAGE *
		get() const
		{
			return &m_message_instance;
		}

		//! Вызвать метод у сообщения.
		const MESSAGE *
		operator -> () const
		{
			return get();
		}

	private:
		//! Ссылка на экземпляр сообщения.
		const MESSAGE & m_message_instance;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
