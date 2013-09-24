/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A local mbox definition.
*/

#if !defined( _SO_5__RT__IMPL__LOCAL_MBOX_HPP_ )
#define _SO_5__RT__IMPL__LOCAL_MBOX_HPP_

#include <so_5/h/types.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/mbox.hpp>

#include <so_5/rt/impl/h/mbox_core.hpp>
#include <so_5/rt/impl/h/message_distributor.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// local_mbox_t
//

//! A class for representing local anonymous mbox.
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
		//! Subscribe first event handler to a mbox's message.
		virtual void
		subscribe_first_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for that message type.
			std::unique_ptr< impl::message_consumer_link_t > &
				message_consumer_link,
			//! The first event caller for that message.
			const event_handler_caller_ref_t &
				event_handler_caller_ref );

		//! Add yet another event handler for a mbox's message.
		/*!
		 * This method is called when agent is subscribing for message
		 * to which it was subscribed earlier.
		*/
		virtual ret_code_t
		subscribe_more_event_handler(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Already existed message consumer for that message.
			impl::message_consumer_link_t * message_consumer_link,
			//! Event caller for that message.
			const event_handler_caller_ref_t & event_handler_caller_ref,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Remove all subscription for message specified.
		/*!
		 * This method is called during agent deregistration when
		 * all agent subscriptions should be removed.
		 */
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const type_wrapper_t & type_wrapper,
			//! Message consumer for agent.
			impl::message_consumer_link_t *
				message_consumer_link );
		//! \}

		//! Deliver message for all subscribers.
		void
		deliver_message(
			const type_wrapper_t & type_wrapper,
			const message_ref_t & message_ref );

	private:
		//! Implementation data.
		impl::mbox_core_ref_t m_mbox_core;

		//! Object lock.
		ACE_RW_Thread_Mutex & m_lock;

		//! Message distributor.
		impl::message_distributor_t m_message_distributor;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
