/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A local mbox definition.
*/

#if !defined( _SO_5__RT__IMPL__LOCAL_MBOX_HPP_ )
#define _SO_5__RT__IMPL__LOCAL_MBOX_HPP_

#include <map>

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
// local_mbox_t
//

//! A class for representing a local anonymous mbox.
class local_mbox_t
	:
		public mbox_t
{
		friend class impl::mbox_core_t;

		explicit local_mbox_t(
			impl::mbox_core_t & mbox_core );

		local_mbox_t(
			impl::mbox_core_t & mbox_core,
			ACE_RW_Thread_Mutex & lock );

	public:
		virtual ~local_mbox_t();

		/*!
		 * \return Empty string.
		 */
		virtual const std::string &
		query_name() const;

	protected:
		//! Subscribe the first event handler to a mbox's message.
		virtual void
		subscribe_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Agent-subcriber.
			agent_t * subscriber,
			//! The first event caller for this message.
			const event_caller_block_ref_t & event_caller );

		//! Remove all subscription for the specified message.
		/*!
		 * This method is called during agent deregistration when
		 * all agent subscriptions should be removed.
		 */
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Agent-subscriber.
			agent_t * subscriber );
		//! \}

		//! Deliver message for all subscribers.
		void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref ) const;

	private:
		//! Implementation data.
		impl::mbox_core_ref_t m_mbox_core;

		//! Object lock.
		ACE_RW_Thread_Mutex & m_lock;

		//! Typedef for map of subscribers to caller blocks.
		typedef std::map<
					agent_t *,
					event_caller_block_ref_t >
				subscribers_to_msg_map_t;

		//! Typedef for map from type_wrapper to callers blocks.
		typedef std::map< type_wrapper_t, subscribers_to_msg_map_t >
				subscribers_to_mbox_map_t;

		//! Map of subscribers to messages.
		subscribers_to_mbox_map_t m_subscribers;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
