/*
 * SObjectizer 5
 */

/*!
 * \since v.5.5.10
 * \file
 * \brief Various traits for MPSC queues.
 */

#pragma once

#include <so_5/h/declspec.hpp>

#include <functional>
#include <memory>
#include <chrono>

namespace so_5 {

namespace disp {

namespace mpsc_queue_traits {

//
// lock_t
//
/*!
 * \since v.5.5.10
 * \brief An interface for lock for MPSC queue.
 */
class SO_5_TYPE lock_t
	{
		friend class unique_lock_t;
		friend class lock_guard_t;

	public :
		lock_t();
		lock_t( const lock_t & ) = delete;
		lock_t( lock_t && ) = delete;
		virtual ~lock_t();

		//! Lock object in exclusive mode.
		virtual void
		lock() = 0;

		//! Unlock object locked in exclusive mode.
		virtual void
		unlock() = 0;

	protected :
		//! Waiting for nofication.
		/*!
		 * \attention Must be called only when object is locked!
		 */
		virtual void
		wait_for_notify() = 0;

		//! Notify one waiting thread if it exists.
		/*!
		 * \attention Must be called only when object is locked.
		 */
		virtual void
		notify_one() = 0;
	};

//
// lock_unique_ptr_t
//
/*!
 * \since v.5.5.10
 * \brief An alias for unique_ptr for lock.
 */
using lock_unique_ptr_t = std::unique_ptr< lock_t >; 

//
// lock_factory_t
//
/*!
 * \since v.5.5.10
 * \brief Type of lock factory.
 */
using lock_factory_t = std::function< lock_unique_ptr_t() >;

//
// combined_lock_factory
//
//FIXME: example of usage is necessary in Doxygen-comment.
/*!
 * \since v.5.5.10
 * \brief Factory for creation of combined queue lock with default waiting time.
 */
SO_5_FUNC lock_factory_t
combined_lock_factory();

//FIXME: example of usage is necessary in Doxygen-comment.
/*!
 * \since v.5.5.10
 * \brief Factory for creation of combined queue lock with the specified
 * waiting time.
 */
SO_5_FUNC lock_factory_t
combined_lock_factory(
	//! Max waiting time for waiting on spinlock before switching to mutex.
	std::chrono::high_resolution_clock::duration waiting_time );

//FIXME: example of usage is necessary in Doxygen-comment.
/*!
 * \since v.5.5.10
 * \brief Factory for creation of very simple implementation based on
 * usage of mutex and condition_variable only.
 */
SO_5_FUNC lock_factory_t
simple_lock_factory();

//
// unique_lock_t
//
/*!
 * \since v.5.5.10
 * \brief An analog of std::unique_lock for MPSC queue lock.
 */
class unique_lock_t
	{
	public :
		inline
		unique_lock_t( lock_t & lock )
			:	m_lock( lock )
			{
				m_lock.lock();
			}

		inline
		~unique_lock_t()
			{
				m_lock.unlock();
			}

		unique_lock_t( const unique_lock_t & ) = delete;
		unique_lock_t( unique_lock_t && ) = delete;

		inline void
		wait_for_notify()
			{
				m_lock.wait_for_notify();
			}

	private :
		lock_t & m_lock;
	};

//
// lock_guard_t
//
/*!
 * \since v.5.4.0
 * \brief An analog of std::lock_guard for MPSC queue lock.
 */
class lock_guard_t
	{
	public :
		inline
		lock_guard_t(
			lock_t & lock )
			:	m_lock( lock )
			{
				m_lock.lock();
			}
		inline
		~lock_guard_t()
			{
				m_lock.unlock();
			}

		lock_guard_t( const lock_guard_t & ) = delete;
		lock_guard_t( lock_guard_t && ) = delete;

		inline void
		notify_one()
			{
				m_lock.notify_one();
			}

	private :
		lock_t & m_lock;
	};

//
// params_t
//
/*!
 * \since v.5.5.10
 * \brief Container for storing parameters for MPSC queue.
 */
class params_t
	{
	public :
		//! Default constructor.
		params_t()
			:	m_lock_factory{ combined_lock_factory() }
			{}
		//! Copy constructor.
		params_t( const params_t & o )
			:	m_lock_factory{ o.m_lock_factory }
			{}
		//! Move constructor.
		params_t( params_t && o )
			:	m_lock_factory{ std::move(o.m_lock_factory) }
			{}

		friend inline void swap( params_t & a, params_t & b )
			{
				using namespace std;
				swap( a.m_lock_factory, b.m_lock_factory );
			}

		//! Copy operator.
		params_t & operator=( const params_t & o )
			{
				params_t tmp{ o };
				swap( *this, tmp );
				return *this;
			}

		//! Move operator.
		params_t & operator=( params_t && o )
			{
				params_t tmp{ std::move(o) };
				swap( *this, tmp );
				return *this;
			}

		//! Setter for lock factory.
		params_t &
		lock_factory( lock_factory_t factory )
			{
				m_lock_factory = std::move( factory );
				return *this;
			}

		//! Getter for lock factory.
		const lock_factory_t &
		lock_factory() const
			{
				return m_lock_factory;
			}

	private :
		//! Lock factory to be used during queue creation.
		lock_factory_t m_lock_factory;
	};

} /* namespace mpsc_queue_traits */

} /* namespace disp */

} /* namespace so_5 */

