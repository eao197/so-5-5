/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Базовый класс для вызова метода обработчика у агента
	и шаблон, который обеспечивает конкретные экземпляры объекта.
*/


#if !defined( _SO_5__RT__EVENT_HANDLER_CALLER_HPP_ )
#define _SO_5__RT__EVENT_HANDLER_CALLER_HPP_

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/message_ref.hpp>

namespace so_5
{

namespace rt
{

class agent_t;
class message_t;


class event_handler_caller_t;

//
// event_handler_caller_t
//

//! Базовый класс для вызова обработчика.
/*!
	Через этот интерфейс подсистемы работают с вызывателями
	обработчиков событий.
*/
class SO_5_TYPE event_handler_caller_t
	:
		private atomic_refcounted_t
{
		friend class event_handler_caller_ref_t;

		event_handler_caller_t(
			const event_handler_caller_t & );

		void
		operator = (
			const event_handler_caller_t & );

	public:
		event_handler_caller_t();
		virtual ~event_handler_caller_t();

		//! Вызвать у агента метод обработки события.
		/*!
			\return Если событие было выполнено, то
			вернет true, в противном случае вернет false.

			Событие не выполняется только в том случае,
			если агент находится в состоянии отличном от
			того с которым он подписывался на сообщение.
		*/
		virtual bool
		call(
			//! Сообщение.
			message_ref_t & message_ref ) const = 0;

		virtual type_wrapper_t
		type_wrapper() const = 0;

		//! Значение для сравнения
		//! (указатель на метод обработки события).
		virtual char *
		ordinal() const = 0;

		//! Размер значения для сравнния.
		virtual size_t
		ordinal_size() const = 0;

		//! Указатель на событие.
		virtual const state_t *
		target_state() const;
};

//
// real_event_handler_caller_t
//

//! Шаблонный класс для вызова обработчика у конкретного агента с
//! конкретным типом сообщения.
template< class MESSAGE, class AGENT >
class real_event_handler_caller_t
	:
		public event_handler_caller_t
{
	public:
		//! Указатель на метод обработчик события агента.
		typedef void (AGENT::*FN_PTR_T)(
			const event_data_t< MESSAGE > & );

		real_event_handler_caller_t(
			FN_PTR_T pfn,
			AGENT & agent,
			const state_t * target_state )
			:
				m_pfn( pfn ),
				m_agent( agent ),
				m_target_state( target_state )
		{}

		virtual ~real_event_handler_caller_t()
		{}

		virtual type_wrapper_t
		type_wrapper() const
		{
			return quick_typeid_t< MESSAGE >::m_type_wrapper;
		}

		//! Значение для сравнения.
		//! Возращает указатель на участок памяти в котором
		//! храниться указатель на метод класса.
		virtual char *
		ordinal() const
		{
			return (char *) &m_pfn;
		}

		//! Размер значения для сравнния.
		virtual size_t
		ordinal_size() const
		{
			return sizeof( FN_PTR_T );
		}

		virtual const state_t *
		target_state() const
		{
			return m_target_state;
		}

		//! Вызвать у агента метод обработки события.
		/*!
			\return Если событие было выполнено, то
			вернет true, в противном случае вернет false.
		*/
		virtual bool
		call(
			//! Сообщение.
			message_ref_t & message_ref ) const
		{
			const bool execute = &m_agent.so_current_state() == m_target_state;

			// Если агент находится в том состоянии, в котором
			// подписан на событие, то вызываем обработчик.
			if( execute )
			{
				const event_data_t< MESSAGE > event_data(
					reinterpret_cast< const MESSAGE * >( message_ref.get() ) );

				(m_agent.*m_pfn)( event_data );
			}

			return execute;
		};

	private:
		//! Состояние в котором должно обрабатываться сообщение.
		const state_t * const m_target_state;

		//! Указатель на метод обработчик события агента.
		FN_PTR_T m_pfn;

		//! Ссылка на агент у которого надо вызывать обработчик.
		AGENT & m_agent;
};

//
// not_null_data_real_event_handler_caller_t
//

//! Шаблонный класс для вызова обработчика у конкретного агента с
//! конкретным типом сообщения.
template< class MESSAGE, class AGENT >
class not_null_data_real_event_handler_caller_t
	:
		public event_handler_caller_t
{
	public:
		//! Указатель на метод обработчик события агента.
		typedef void (AGENT::*FN_PTR_T)(
			const not_null_event_data_t< MESSAGE > & );

		not_null_data_real_event_handler_caller_t(
			FN_PTR_T pfn,
			AGENT & agent,
			const state_t * target_state )
			:
				m_pfn( pfn ),
				m_agent( agent ),
				m_target_state( target_state )
		{}

		virtual ~not_null_data_real_event_handler_caller_t()
		{}


		virtual type_wrapper_t
		type_wrapper() const
		{
			return quick_typeid_t< MESSAGE >::m_type_wrapper;
		}

		//! Значение для сравнения.
		//! Возращает указатель на участок памяти, в котором
		//! храниться указатель на метод класса.
		virtual char *
		ordinal() const
		{
			return (char *) &m_pfn;
		}

		//! Размер значения для сравнния.
		virtual size_t
		ordinal_size() const
		{
			return sizeof( FN_PTR_T );
		}

		virtual const state_t *
		target_state() const
		{
			return m_target_state;
		}

		//! Вызвать у агента метод обработки события.
		/*!
			\return Если событие было выполнено, то
			вернет true, в противном случае вернет false.
		*/
		virtual bool
		call(
			//! Сообщение.
			message_ref_t & message_ref ) const
		{
			bool execute = &m_agent.so_current_state() == m_target_state;

			// Если агент находится в том состоянии, в котором
			// подписан на событие, то вызываем обработчик.
			if( execute )
			{
				if( message_ref.get() )
				{
					const not_null_event_data_t< MESSAGE > not_null_event_data(
						reinterpret_cast< const MESSAGE & >( *message_ref ) );

					(m_agent.*m_pfn)( not_null_event_data );
				}
			}

			return execute;
		};

	private:
		//! Состояние в котором должно обрабатываться сообщение.
		const state_t * const m_target_state;

		//! Указатель на метод обработчик события агента.
		FN_PTR_T m_pfn;

		//! Ссылка на агент, у которого надо вызывать обработчик.
		AGENT & m_agent;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
