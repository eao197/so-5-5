/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.4.0
	\brief A proxy for event_queue pointer.
*/

#if !defined( _SO_5__RT__EVENT_QUEUE_PROXY_HPP_ )
#define _SO_5__RT__EVENT_QUEUE_PROXY_HPP_

#include <so_5/rt/h/event_queue.hpp>
#include <so_5/rt/h/atomic_refcounted.hpp>

namespace so_5
{

namespace rt
{

/*!
 * \since v.5.4.0
 * \brief A proxy for event_queue pointer.
 */
class event_queue_proxy_t : private atomic_refcounted_t
	{
		friend class smart_atomic_reference_t< event_queue_proxy_t >;

	public :
		event_queue_proxy_t()
			{
				m_lock.clear( std::memory_order_release );
				m_workers = 0;
				m_queue_ptr = nullptr;
			}

		//! Switch to the specified queue.
		inline void
		switch_to( event_queue_t & queue )
			{
				setup_queue_ptr( &queue );
			}

		//! Shutdown proxy object.
		/*!
		 * \return last value of event_queue pointer.
		 */
		inline event_queue_t *
		shutdown()
			{
				return setup_queue_ptr( nullptr );
			}

		//! Enqueue new event to the queue.
		inline void
		push( execution_demand_t demand )
			{
				lock();

				event_queue_t * q = m_queue_ptr.load( std::memory_order_consume );
				if( q )
					{
						workers_count_guard_t guard( m_workers );
						unlock();

						q->push( std::move( demand ) );
					}
				else
					unlock();
			}

	private :
		//! Object's lock.
		/*!
		 * Is used as a spinlock.
		 */
		std::atomic_flag m_lock;

		//! Current workers count.
		std::atomic_uint m_workers;

		//! A pointer to the actual event_queue.
		/*!
		 * nullptr value means that event_queue is shut down.
		 */
		std::atomic< event_queue_t * > m_queue_ptr;

		//! A helper class for insure that workers count increments
		//! are in pair with decrements.
		struct workers_count_guard_t
			{
				std::atomic_uint & m_value;

				inline workers_count_guard_t( std::atomic_uint & v )
					:	m_value( v )
					{
						++m_value;
					}
				inline ~workers_count_guard_t()
					{
						--m_value;
					}
			};

		//! Lock object.
		inline void
		lock()
			{
				while( m_lock.test_and_set( std::memory_order_acquire ) )
					;
			}

		//! Unlock object.
		inline void
		unlock()
			{
				m_lock.clear( std::memory_order_release );
			}

		//! Wait until all worker finished their work.
		inline void
		wait_all_workers_gone()
			{
				while( 0 != m_workers.load( std::memory_order_acquire ) )
					;
			}

		//! Setup a pointer to queue.
		inline event_queue_t *
		setup_queue_ptr( event_queue_t * queue )
			{
				lock();
				wait_all_workers_gone();

				auto r = m_queue_ptr.load( std::memory_order_consume );
				m_queue_ptr.store( queue, std::memory_order_release );

				unlock();

				return r;
			}
	};

/*!
 * \since v.5.4.0
 * \brief Typedef for smart pointer to event_queue_proxy.
 */
typedef smart_atomic_reference_t< event_queue_proxy_t >
		event_queue_proxy_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

