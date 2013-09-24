/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Smart reference to the event_caller_block definition.
*/

#if !defined( _SO_5__RT__EVENT_CALLER_BLOCK_REF_HPP_ )
#define _SO_5__RT__EVENT_CALLER_BLOCK_REF_HPP_

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class event_caller_block_t;

//
// event_caller_block_ref_t
//

//! Smart reference to the event_caller_block.
class SO_5_TYPE event_caller_block_ref_t
{
	public:
		explicit event_caller_block_ref_t(
			event_caller_block_t * event_caller_block_ref_t );

		event_caller_block_ref_t();

		event_caller_block_ref_t(
			const event_caller_block_ref_t &
				event_caller_block_ref );

		void
		operator = (
			const event_caller_block_ref_t &
				event_caller_block_ref );

		~event_caller_block_ref_t();

		event_caller_block_t *
		get() const;

		event_caller_block_t *
		operator -> ();

		const event_caller_block_t *
		operator -> () const;

		event_caller_block_t &
		operator * ();

		const event_caller_block_t &
		operator * () const;

		void
		release();

	private:
		//! Decrement reference count.
		/*!
		 * Deletes event_caller_block if reference counter become 0.
		 */
		void
		dec_event_caller_block_ref_count();

		//! Increment reference count.
		void
		inc_event_caller_block_ref_count();

		//! Event caller block.
		event_caller_block_t * m_event_caller_block_ptr;
};

} /* namespace rt */

} /* namespace so_5 */

#endif

