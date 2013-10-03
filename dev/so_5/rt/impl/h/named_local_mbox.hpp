/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of the named local mbox.
*/

#if !defined( _SO_5__RT__IMPL__NAMED_LOCAL_MBOX_HPP_ )
#define _SO_5__RT__IMPL__NAMED_LOCAL_MBOX_HPP_

#include <so_5/h/types.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/mbox.hpp>

#include <so_5/rt/impl/h/mbox_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// named_local_mbox_t
//

//! A class for the named local mbox.
/*!
 * \note This class is necessary because there is a difference between
 * reference counting for anonymous and named local mboxes. Named
 * local mboxes should have only one instance inside
 * SObjectizer Environment.
*/
class named_local_mbox_t
	:
		public mbox_t
{
		friend class impl::mbox_core_t;

		named_local_mbox_t(
			const std::string & name,
			const mbox_ref_t & mbox,
			impl::mbox_core_t & mbox_core );

	public:
		virtual ~named_local_mbox_t();

		virtual const std::string &
		query_name() const;

	protected:
		virtual void
		subscribe_event_handler(
			const type_wrapper_t & type_wrapper,
			agent_t * subscriber,
			const event_caller_block_ref_t & event_caller );

		virtual void
		unsubscribe_event_handlers(
			const type_wrapper_t & type_wrapper,
			agent_t * subscriber );
		//! \}

		//! Deliver message to all consumers.
		void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

		//! Get data for a comparision.
		virtual const mbox_t *
		cmp_ordinal() const;

	private:
		//! Mbox name.
		const std::string m_name;

		//! An utility for this mbox.
		impl::mbox_core_ref_t m_mbox_core;

		//! Actual mbox.
		mbox_ref_t m_mbox;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
