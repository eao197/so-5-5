/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \file
 * \brief Various locks for using with event queues.
 */

#pragma once

#include <so_5/h/spinlocks.hpp>

#include <mutex>
#include <condition_variable>

namespace so_5
{

namespace disp
{

namespace reuse
{

namespace locks
{

//
// combined_queue_lock_t
//
/*!
 * \since v.5.4.0
 * \brief A special combined lock for queue protection.
 *
 * This lock used spinlocks for efficiency and std::mutex and
 * std::condition_variable for signalization.
 */
class combined_queue_lock_t
	{
		friend class combined_queue_unique_lock_t;
		friend class combined_queue_lock_guard_t;

	public :
		inline
		combined_queue_lock_t()
			:	m_waiting( false )
			,	m_signaled( false )
			{}

		combined_queue_lock_t( const combined_queue_lock_t & ) = delete;
		combined_queue_lock_t( combined_queue_lock_t && ) = delete;

		//! Lock object in exclusive mode.
		inline void
		lock()
			{
				m_spinlock.lock();
			}

		//! Unlock object locked in exclusive mode.
		inline void
		unlock()
			{
				m_spinlock.unlock();
			}

	protected :
		//! Waiting for nofication.
		/*!
		 * \attention Must be called only when object is locked!
		 */
		inline void
		wait_for_notify()
			{
				m_waiting += 1;
				auto stop_point = std::chrono::high_resolution_clock::now() +
						std::chrono::milliseconds(1);

				do
					{
						m_spinlock.unlock();

						std::this_thread::yield();

						m_spinlock.lock();

						if( m_signaled )
							{
								m_waiting -= 1;
								m_signaled -= 1;
								return;
							}
					}
				while( stop_point > std::chrono::high_resolution_clock::now() );

				// m_lock is locked now.

				// Must use heavy std::mutex and std::condition_variable
				// to allow OS to efficiently use the resources while
				// we are waiting for signal.
				std::unique_lock< std::mutex > mlock( m_mutex );

				m_spinlock.unlock();

				m_condition.wait( mlock );

				// At this point m_signaled must be 'true'.

				m_spinlock.lock();

				m_waiting -= 1;
				m_signaled -= 1;
			}

		//! Notify one waiting thread if it exists.
		/*!
		 * \attention Must be called only when object is locked.
		 */
		inline void
		notify_one()
			{
				if( m_waiting )
					{
						// There is a waiting thread.
						m_mutex.lock();
						m_signaled += 1;
						m_condition.notify_one();
						m_mutex.unlock();
					}
			}

		//! Notify all waiting threads if exist.
		/*!
		 * \attention Must be called only when object is locked.
		 */
		inline void
		notify_all()
			{
				if( m_waiting )
					{
						// There is a waiting thread.
						m_mutex.lock();
						m_signaled = m_waiting;
						m_condition.notify_all();
						m_mutex.unlock();
					}
			}

	private :
		default_spinlock_t m_spinlock;

		std::mutex m_mutex;
		std::condition_variable m_condition;

		std::uint_fast32_t m_waiting;
		std::uint_fast32_t m_signaled;
	};

//
// combined_queue_unique_lock_t
//
/*!
 * \since v.5.4.0
 * \brief An analog of std::unique_lock for combined_queue_lock.
 */
class combined_queue_unique_lock_t
	{
	public :
		inline
		combined_queue_unique_lock_t(
			combined_queue_lock_t & lock )
			:	m_lock( lock )
			{
				m_lock.lock();
			}
		inline
		~combined_queue_unique_lock_t()
			{
				m_lock.unlock();
			}

		combined_queue_unique_lock_t( const combined_queue_unique_lock_t & )
			= delete;
		combined_queue_unique_lock_t( combined_queue_unique_lock_t && )
			= delete;

		inline void
		wait_for_notify()
			{
				m_lock.wait_for_notify();
			}

	private :
		combined_queue_lock_t & m_lock;
	};

//
// combined_queue_lock_guard_t
//
/*!
 * \since v.5.4.0
 * \brief An analog of std::lock_guard for combined_queue_lock.
 */
class combined_queue_lock_guard_t
	{
	public :
		inline
		combined_queue_lock_guard_t(
			combined_queue_lock_t & lock )
			:	m_lock( lock )
			{
				m_lock.lock();
			}
		inline
		~combined_queue_lock_guard_t()
			{
				m_lock.unlock();
			}

		combined_queue_lock_guard_t( const combined_queue_lock_guard_t & )
			= delete;
		combined_queue_lock_guard_t( combined_queue_lock_guard_t && )
			= delete;

		inline void
		notify_one()
			{
				m_lock.notify_one();
			}

		inline void
		notify_all()
			{
				m_lock.notify_all();
			}

	private :
		combined_queue_lock_t & m_lock;
	};

} /* namespace locks */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

