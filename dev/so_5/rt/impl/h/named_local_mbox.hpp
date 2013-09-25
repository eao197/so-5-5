/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of named local mbox.
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

//! A class for named local mbox.
/*!
 * This class in necessary because there is a difference between
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
		//! Add a first event handler for a consumer.
		virtual void
		subscribe_first_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! A consumer to be added.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! Event handler to be added to consumer.
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		//! Add another event handler for a consumer.
		virtual ret_code_t
		subscribe_more_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer to be modified.
			impl::message_consumer_link_t * message_consumer_link,
			//! Event handler to be added to consumer.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Remove all consumer subscriptions.
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Consumer to be removed.
			impl::message_consumer_link_t *
				message_consumer_link );
		//! \}

		//! Deliver message to all consumers.
		void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

		//! Get data for comparision.
		virtual const mbox_t *
		cmp_ordinal() const;

	private:
		//! Mbox name.
		const std::string m_name;

		//! An utility for that mbox.
		impl::mbox_core_ref_t m_mbox_core;

		//! Actual mbox.
		mbox_ref_t m_mbox;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
