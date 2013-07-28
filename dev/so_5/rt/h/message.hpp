/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Базовый класс для сообщений.
*/

#if !defined( _SO_5__RT__MESSAGE_HPP_ )
#define _SO_5__RT__MESSAGE_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/atomic_refcounted.hpp>

namespace so_5
{

namespace rt
{

//
// message_t
//

//! Базовый класс для сообщений.
/*!
	Все сооббщения, посылаемые через mbox-ы должны
	быть нвследниками данного класса.
*/
class SO_5_TYPE message_t
	:
		private atomic_refcounted_t
{
		friend class message_ref_t;

	public:
		message_t();
		message_t( const message_t & );
		void
		operator = ( const message_t & );

		virtual ~message_t();

		//! Проверить данные в сообщении.
		/*!
			\return true если сообщение прошло проверку.
			false если сообщение не проходит проверку.

			\note Реализация по умолчанию всегда возвращает
			true.
		*/
		virtual bool
		check() const;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
