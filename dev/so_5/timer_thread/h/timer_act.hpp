/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Таймерное событие.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_ACT_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_ACT_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/message_ref.hpp>

namespace so_5
{

namespace timer_thread
{

//
// timer_act_t
//

//! Таймерное событие.
class SO_5_TYPE timer_act_t
{
		timer_act_t( const timer_act_t & );
		void
		operator = ( const timer_act_t & );

	public:
		timer_act_t(
			//! Тип сообщения.
			const rt::type_wrapper_t & type_wrapper,
			//! mbox, на который надо отправлять периодическое сообщение.
			const rt::mbox_ref_t & mbox,
			//! Экземпляр сообщения.
			const rt::message_ref_t & msg,
			/*! Для переодического сообщения указывает
				время, через которое сообщение должно
				возникнуть в первый раз. */
			unsigned int delay,
			/*! Период, отличен от 0 для переодических сообщений. */
			unsigned int period );

		~timer_act_t();

		//! Выполнить действие.
		void
		exec();

		//! Является ли таймерное действие периодическим?
		bool
		is_periodic() const;

		//! Задержка для первого раза.
		unsigned int
		query_delay() const;

		/*! Период, отличен от 0 для переодических сообщений. */
		unsigned int
		query_period() const;

	private:
		//! Тип сообщения.
		const rt::type_wrapper_t m_type_wrapper;

		//! mbox, на который надо отправлять периодическое сообщение.
		rt::mbox_ref_t m_mbox;

		//! Экземпляр сообщения.
		const rt::message_ref_t m_msg;

		/*! Для переодического сообщения указывает
			время, через которое сообщение должно
			возникнуть в первый раз. */
		const unsigned int m_delay;

		/*! Период, отличен от 0 для переодических сообщений. */
		const unsigned int m_period;
};

//! Псевдоним unique_ptr для timer_act_t.
typedef std::unique_ptr< timer_act_t > timer_act_unique_ptr_t;

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
