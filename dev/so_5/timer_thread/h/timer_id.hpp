/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Ссылка на сообщение.
*/

#if !defined( _SO_5__TIMER_THREAD__TIMER_ID_REF_HPP_ )
#define _SO_5__TIMER_THREAD__TIMER_ID_REF_HPP_

#include <ace/Basic_Types.h>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>

namespace so_5
{

namespace timer_thread
{

//! Идентификатор таймерного события SObjectizer.
typedef ACE_UINT64 timer_id_t;

class timer_thread_t;
class timer_id_ref_t;

//
// timer_id_internals_t
//

//! Обертка над timer_id_t, которая удаляет событие в случае
//! собственного уничтожения.
class timer_id_internals_t
	:
		private so_5::rt::atomic_refcounted_t
{
		friend class timer_id_ref_t;

		timer_id_internals_t( const timer_id_internals_t & );
		void
		operator = ( const timer_id_internals_t & );

	public:

		timer_id_internals_t(
			//! Таймерная нить.
			timer_thread_t & timer_thread,
			//! Идентификатор таймерного события.
			timer_id_t timer_id );

		virtual ~timer_id_internals_t();

	private:
		//! Ссылка на таймерную нить.
		timer_thread_t & m_timer_thread;

		//! Идентификатор таймерного события.
		timer_id_t m_timer_id;
};

//
// timer_id_ref_t
//

//! Ref - обертка над timer_id_t.
class SO_5_TYPE timer_id_ref_t
{
		explicit timer_id_ref_t(
			timer_id_internals_t * timer_id_internals );
	public:

		static timer_id_ref_t
		create(
			//! Таймерная нить.
			timer_thread_t & timer_thread,
			//! Идентификатор таймерного события.
			timer_id_t timer_id );

		timer_id_ref_t();

		timer_id_ref_t( const timer_id_ref_t & timer_id );

		~timer_id_ref_t();

		timer_id_ref_t &
		operator = ( const timer_id_ref_t & timer_id );

		//! Проверить содержится ли в timer_id_ref_t активное событие.
		bool
		is_active() const;

		//! Отпустить таймерное событие.
		void
		release();

	private:
		//! Увеличить количество ссылок на объект.
		void
		inc_timer_id_ref_count();

		//! Уменьшить количество ссылок на объект.
		void
		dec_timer_id_ref_count();

		//! Хранитель таймерного события.
		timer_id_internals_t * m_timer_id_internals;
};

} /* namespace timer_thread */

} /* namespace so_5 */

#endif
