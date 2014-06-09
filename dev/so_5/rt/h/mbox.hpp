/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Mbox definition.
*/


#if !defined( _SO_5__RT__MBOX_HPP_ )
#define _SO_5__RT__MBOX_HPP_

#include <string>
#include <memory>
#include <typeindex>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/h/declspec.hpp>

#include <so_5/h/exception.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/message.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

namespace so_5
{

namespace timer_thread
{
	class timer_act_t;
}

namespace rt
{

namespace impl
{

class message_consumer_link_t;
class named_local_mbox_t;

} /* namespace impl */

class agent_t;

//
// mbox_t
//

//! Mail box class.
/*!
 * The class serves as an interface for sending and receiving messages.
 *
 * All mboxes can be created via the SObjectizer Environment. References to
 * mboxes are stored and manipulated by so_5::rt::mbox_ref_t objects.
 *
 * mbox_t has two versions of the deliver_message() method. 
 * The first one requires pointer to the actual message data and is intended 
 * for delivering messages to agents.
 * The second one doesn't use a pointer to the actual message data and 
 * is intended for delivering signals to agents.
 *
 * mbox_t also is used for the delivery of delayed and periodic messages.
 * The SObjectizer Environment stores mbox for which messages must be
 * delivered and the timer thread pushes message instances to the mbox
 * at the appropriate time.
 *
 * \see so_environment_t::schedule_timer(), so_environment_t::single_timer().
 */
class SO_5_TYPE mbox_t
	:
		private atomic_refcounted_t
{
		friend class impl::named_local_mbox_t;
		friend class so_5::timer_thread::timer_act_t;

		friend class smart_atomic_reference_t< mbox_t >;

		friend class mbox_subscription_management_proxy_t;

		mbox_t( const mbox_t & );
		void
		operator = ( const mbox_t & );

	public:
		mbox_t();
		virtual ~mbox_t();

		//! Deliver message.
		/*!
		 * \since v.5.2.2
		 *
		 * Mbox takes care about destroying a message object.
		 */
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			const smart_atomic_reference_t< MESSAGE > & msg_ref ) const;

		//! Deliver message.
		/*!
		 * Mbox takes care about destroying a message object.
		 */
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			std::unique_ptr< MESSAGE > msg_unique_ptr ) const;

		//! Deliver message.
		/*!
		 * Mbox takes care about destroying a message object.
		 */
		template< class MESSAGE >
		inline void
		deliver_message(
			//! Message data.
			MESSAGE * msg_raw_ptr ) const;

		//! Deliver signal.
		template< class MESSAGE >
		inline void
		deliver_signal() const;

		/*!
		 * \since v.5.3.0.
		 * \brief Deliver service request.
		 */
		virtual void
		deliver_service_request(
			//! This is type_index for service PARAM type.
			const std::type_index & type_index,
			//! This is reference to msg_service_request_t<RESULT,PARAM> instance.
			const message_ref_t & svc_request_ref ) const = 0;

		//! Get the mbox name.
		virtual const std::string &
		query_name() const = 0;

		/*!
		 * \name Comparision.
		 * \{
		 */
		bool operator==( const mbox_t & o ) const;

		bool operator<( const mbox_t & o ) const;
		/*!
		 * \}
		 */

	protected:
		//! Add the message handler.
		/*!
		 * This method is called when the agent is subscribing to the message
		 * at the first time.
		 */
		virtual void
		subscribe_event_handler(
			//! Message type.
			const std::type_index & type_index,
			//! Agent-subcriber.
			agent_t * subscriber,
			//! The very first message handler.
			const event_caller_block_ref_t & event_caller ) = 0;

		//! Remove all message handlers.
		virtual void
		unsubscribe_event_handlers(
			//! Message type.
			const std::type_index & type_index,
			//! Agent-subcriber.
			agent_t * subscriber ) = 0;

		//! Deliver message for all subscribers.
		virtual void
		deliver_message(
			const std::type_index & type_index,
			const message_ref_t & message_ref ) const = 0;

		/*!
		 * \since v.5.2.3.4
		 * \brief Lock mbox in read-write mode.
		 */
		virtual void
		read_write_lock_acquire() = 0;

		/*!
		 * \since v.5.2.3.4
		 * \brief Release mbox's read-write lock.
		 */
		virtual void
		read_write_lock_release() = 0;

		//! Get data for the object comparision.
		/*!
		 * Default implementation returns this pointer.
		*/
		virtual const mbox_t *
		cmp_ordinal() const;
};

template< class MESSAGE >
inline void
mbox_t::deliver_message(
	const smart_atomic_reference_t< MESSAGE > & msg_ref ) const
{
	ensure_message_with_actual_data( msg_ref.get() );

	deliver_message(
		std::type_index( typeid( MESSAGE ) ),
		msg_ref.template make_reference< message_t >() );
}

template< class MESSAGE >
void
mbox_t::deliver_message(
	std::unique_ptr< MESSAGE > msg_unique_ptr ) const
{
	ensure_message_with_actual_data( msg_unique_ptr.get() );

	deliver_message(
		std::type_index( typeid( MESSAGE ) ),
		message_ref_t( msg_unique_ptr.release() ) );
}

template< class MESSAGE >
void
mbox_t::deliver_message(
	MESSAGE * msg_raw_ptr ) const
{
	this->deliver_message( std::unique_ptr< MESSAGE >( msg_raw_ptr ) );
}

template< class MESSAGE >
void
mbox_t::deliver_signal() const
{
	ensure_signal< MESSAGE >();

	deliver_message(
		std::type_index( typeid( MESSAGE ) ),
		message_ref_t() );
}

//
// mbox_ref_t
//
//! Smart reference for the mbox_t.
/*!
 * \note Defined as typedef since v.5.2.0
 */
typedef smart_atomic_reference_t< mbox_t > mbox_ref_t;

/*!
 * \since v.5.3.0
 * \brief A special proxy for service request invocation.
 */
template< class RESULT >
class service_invoke_proxy_t
	{
	public :
		service_invoke_proxy_t( const mbox_t & mbox )
			:	m_mbox( mbox )
			{}

		//! Make asynchronous service request.
		/*!
		 * This method should be used for the cases where PARAM is a signal.
		 */
		template< class PARAM >
		std::future< RESULT >
		request() const
			{
				ensure_signal< PARAM >();

				std::promise< RESULT > promise;
				auto f = promise.get_future();

				message_ref_t ref(
						new msg_service_request_t< RESULT, PARAM >(
								std::move(promise) ) );
				m_mbox.deliver_service_request(
						std::type_index( typeid(PARAM) ),
						ref );

				return f;
			}

		//! Make synchronous service request call.
		/*!
		 * This method should be used for the case where PARAM is a signal.
		 */
//FIXME: there should be timeout as second parameter!
		template< class PARAM >
		RESULT
		sync_request() const
			{
				return this->request< PARAM >().get();
			}

		//! Make call for service request with param.
		/*!
		 * This method should be used for the case where PARAM is a message.
		 */
		template< class PARAM >
		std::future< RESULT >
		request( smart_atomic_reference_t< PARAM > msg_ref ) const
			{
				ensure_message_with_actual_data( msg_ref.get() );

				std::promise< RESULT > promise;
				auto f = promise.get_future();

				message_ref_t ref(
						new msg_service_request_t< RESULT, PARAM >(
								std::move(promise),
								msg_ref.template make_reference< message_t >() ) );

				m_mbox.deliver_service_request(
						std::type_index( typeid(PARAM) ),
						ref );

				return f;
			}

		//! Make call for service request with param.
		/*!
		 * This method should be used for the case where PARAM is a message.
		 */
		template< class PARAM >
		std::future< RESULT >
		request( std::unique_ptr< PARAM > msg_unique_ptr ) const
			{
				return this->request( smart_atomic_reference_t< PARAM >(
							msg_unique_ptr.release() ) );
			}

		//! Make call for service request with param.
		/*!
		 * This method should be used for the case where PARAM is a message.
		 */
		template< class PARAM >
		std::future< RESULT >
		request( PARAM * msg ) const
			{
				return this->request( smart_atomic_reference_t< PARAM >( msg ) );
			}

		//! Make synchronous service request call with parameter.
		/*!
		 * This method should be used for the case where PARAM is a message.
		 */
//FIXME: there should be timeout as second parameter!
		template< class PARAM >
		RESULT
		sync_request( std::unique_ptr< PARAM > msg_unique_ptr ) const
			{
				return this->request( std::move(msg_unique_ptr) ).get();
			}

		//! Make synchronous service request call with parameter.
		/*!
		 * This method should be used for the case where PARAM is a message.
		 */
//FIXME: there should be timeout as second parameter!
		template< class PARAM >
		RESULT
		sync_request( PARAM * msg ) const
			{
				return this->sync_request( std::unique_ptr< PARAM >( msg ) );
			}

	private :
		const mbox_t & m_mbox;
	};

/*!
 * \since v.5.3.0
 * \brief Create a special proxy for service request invocation.
 */
template< class RESULT >
inline service_invoke_proxy_t< RESULT >
service( const mbox_ref_t & mbox )
	{
		return service_invoke_proxy_t< RESULT >( *mbox );
	}

/*!
 * \since v.5.2.3.4
 * \brief A special interface to perform subscription management.
 *
 * Mbox should be locked in read-write mode before making any
 * changes to subscriptions. This interface provides an approach
 * to do that without possibility to make some error with object
 * locking/unlocking.
 */
class mbox_subscription_management_proxy_t
{
	public :
		inline mbox_subscription_management_proxy_t(
			const mbox_ref_t & mbox )
			:	m_mbox( mbox )
		{
			m_mbox->read_write_lock_acquire();
		}
		inline ~mbox_subscription_management_proxy_t()
		{
			m_mbox->read_write_lock_release();
		}

		//! Add the message handler.
		inline void
		subscribe_event_handler(
			//! Message type.
			const std::type_index & type_index,
			//! Agent-subcriber.
			agent_t * subscriber,
			//! The very first message handler.
			const event_caller_block_ref_t & event_caller )
		{
			m_mbox->subscribe_event_handler(
					type_index,
					subscriber,
					event_caller );
		}

		//! Remove all message handlers.
		inline void
		unsubscribe_event_handlers(
			//! Message type.
			const std::type_index & type_index,
			//! Agent-subcriber.
			agent_t * subscriber )
		{
			m_mbox->unsubscribe_event_handlers( type_index, subscriber );
		}

	private :
		//! Mbox to work with.
		const mbox_ref_t m_mbox;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
