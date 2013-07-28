/*
	SObjectizer 5.
*/

/*!
	\file
	\brief  ласс ссылка на event_handler_caller_t.
*/

#if !defined( _SO_5__RT__EVENT_HANDLER_CALLER_REF_HPP_ )

#define _SO_5__RT__EVENT_HANDLER_CALLER_REF_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class agent_t;
class event_handler_caller_t;

//! —сылка на event_handler_caller_t.
class SO_5_TYPE event_handler_caller_ref_t
{
	public:
		explicit event_handler_caller_ref_t(
			event_handler_caller_t * event_handler_caller );

		event_handler_caller_ref_t();

		event_handler_caller_ref_t(
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		void
		operator = (
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		~event_handler_caller_ref_t();

		event_handler_caller_t *
		get() const;

		event_handler_caller_t *
		operator -> ();

		const event_handler_caller_t *
		operator -> () const;

		event_handler_caller_t &
		operator * ();

		const event_handler_caller_t &
		operator * () const;

		void
		release();

		bool
		operator == ( const event_handler_caller_ref_t &
			event_handler_caller_ref ) const;

	private:
		//! ”меньшить количество ссылок на event_handler_caller
		//! и в случае необходимости удалить его.
		void
		dec_event_handler_caller_ref_count();

		//! ”величить количество ссылок на event_handler_caller.
		void
		inc_event_handler_caller_ref_count();

		//! ”казатель на event_handler_caller.
		event_handler_caller_t * m_event_handler_caller_ptr;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
