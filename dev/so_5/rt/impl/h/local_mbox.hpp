/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A local mbox definition.
*/

#if !defined( _SO_5__RT__IMPL__LOCAL_MBOX_HPP_ )
#define _SO_5__RT__IMPL__LOCAL_MBOX_HPP_

#include <set>

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
			impl::mbox_core_t & mbox_core,
			mbox_id_t id );

		local_mbox_t(
			impl::mbox_core_t & mbox_core,
			mbox_id_t id,
			ACE_RW_Thread_Mutex & lock );

	public:
		virtual ~local_mbox_t();

		virtual mbox_id_t
		id() const
			{
				return m_id;
			}

		void
		deliver_message(
			const std::type_index & type_wrapper,
			const message_ref_t & message_ref ) const;

		virtual void
		deliver_service_request(
			const std::type_index & type_index,
			const message_ref_t & svc_request_ref ) const;

		//! Subscribe the first event handler to a mbox's message.
		virtual void
		subscribe_event_handler(
			//! Message type.
			const std::type_index & type_wrapper,
			//! Agent-subcriber.
			agent_t * subscriber );

		//! Remove all subscription for the specified message.
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const std::type_index & type_wrapper,
			//! Agent-subscriber.
			agent_t * subscriber );

		//! Name of the mbox.
		virtual std::string
		query_name() const;

	private:
		//! Implementation data.
		impl::mbox_core_ref_t m_mbox_core;

		/*!
		 * \since v.5.4.0
		 * \brief ID of this mbox.
		 */
		const mbox_id_t m_id;

		//! Object lock.
		ACE_RW_Thread_Mutex & m_lock;

		//! Typedef for set of subscribers.
		typedef std::set< std::pair< std::type_index, agent_t * > >
				subscribers_set_t;

		//! Map of subscribers to messages.
		subscribers_set_t m_subscribers;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
