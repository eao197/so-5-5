/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Working thread for dispatchers.
*/

#if !defined( _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_ )
#define _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <so_5/h/declspec.hpp>
#include <so_5/h/current_thread_id.hpp>

#include <so_5/rt/h/event_queue.hpp>

#include <so_5/h/spinlocks.hpp>

namespace so_5
{

namespace disp
{

namespace reuse
{

namespace work_thread
{

//! Typedef for demand's container.
typedef std::deque< so_5::rt::execution_demand_t > demand_container_t;

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
				m_waiting = true;
				auto stop_point = std::chrono::high_resolution_clock::now() +
						std::chrono::milliseconds(1);

				do
					{
						m_spinlock.unlock();

						std::this_thread::yield();

						m_spinlock.lock();

						if( m_signaled )
							{
								m_waiting = false;
								m_signaled = false;
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

				m_waiting = false;
				m_signaled = false;
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
						if( !m_signaled )
							{
								m_signaled = true;
								m_condition.notify_one();
							}
						m_mutex.unlock();
					}
			}

	private :
		default_spinlock_t m_spinlock;

		std::mutex m_mutex;
		std::condition_variable m_condition;

		bool m_waiting;
		bool m_signaled;
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

	private :
		combined_queue_lock_t & m_lock;
	};

//
// demand_queue_t
//

//! Queue of demands to process agent events.
/*!
	demand_queue_t has shutdown flag inside.

	demand_queue_t is thread safe and is intended to be used by 
	several concurrent threads.
*/
class demand_queue_t : public so_5::rt::event_queue_t
{
	public:
		demand_queue_t();
		~demand_queue_t();

		/*!
		 * \name Implementation of event_queue interface.
		 * \{
		 */
		virtual void
		push(
			so_5::rt::execution_demand_t demand );
		/*!
		 * \}
		 */
		enum
		{
			//! Demand has been extracted.
			demand_extracted = 1,
			//! Demand has not been extracted because of shutdown.
			shutting_down = 2,
			//! Demand has not been extracted because the demand queue is empty.
			no_demands = 3
		};

		//! Try to extract demands from the queue.
		/*!
			If there is no demands in queue then current thread
			will sleep until:
			- the new demand is put in the queue;
			- a shutdown signal.
		*/
		int
		pop(
			/*! Receiver for extracted demands. */
			demand_container_t & queue_item );

		//! Start demands processing.
		void
		start_service();

		//! Stop demands processing.
		void
		stop_service();

		//! Clear demands queue.
		void
		clear();

	private:
		//! Demand queue.
		demand_container_t m_demands;

		//! \name Objects for the thread safety.
		//! \{
		combined_queue_lock_t m_lock;
		//! \}

		//! Service flag.
		/*!
			true -- shall do the service, methods push/pop must work.
			false -- the service is stopped or will be stopped.
		*/
		bool m_in_service;
};

//
// work_thread_t
//

//! Working thread.
/*!
 * Working thread should be used inside of some dispatcher.
 * And the lifetime of the dispatcher object must be longer than
 * the lifetime of the working thread object.
 */
class work_thread_t
{
	public:
		work_thread_t(
			rt::dispatcher_t & disp );

		~work_thread_t();

		//! Start the working thread.
		void
		start();

		//! Send the shutdown signal to the working thread.
		void
		shutdown();

		//! Wait the full stop of the working thread.
		/*!
		 * All non-processed demands from the queue will be destroyed
		 * after a stop of the working thread.
		 */
		void
		wait();

		/*!
		 * \since v.5.4.0
		 * \brief Get the underlying event_queue object.
		 */
		so_5::rt::event_queue_t &
		event_queue();

		/*!
		 * \since v.5.4.0
		 * \brief Get the working thread ID.
		 *
		 * \attention This method must be called only on running thread.
		 */
		so_5::current_thread_id_t
		thread_id();

		/*!
		 * \since v.5.4.0
		 * \brief Get a binding information for an agent.
		 */
		std::pair< so_5::current_thread_id_t, so_5::rt::event_queue_t * >
		get_agent_binding();

	protected:
		//! Main working thread body.
		void
		body();

		//! Exception handler.
		void
		handle_exception(
			//! Raised and caught exception.
			const std::exception & ex,
			//! Agent who is the producer of the exception.
			so_5::rt::agent_t & a_exception_producer );

		/*!
		 * \since v.5.2.3
		 * \brief Exception handler for the case when exception caught
		 * but there is no current working agent.
		 */
		void
		handle_exception_on_empty_demands_queue(
			//! Raised and caught exception.
			const std::exception & ex );

		/*!
		 * \since v.5.2.3
		 * \brief Log unhandled exception from cooperation.
		 *
		 * Calls abort() if an exception is raised during logging.
		 */
		void
		log_unhandled_exception(
			//! Raised and caught exception.
			const std::exception & ex,
			//! Agent who is the producer of the exception.
			so_5::rt::agent_t & a_exception_producer );

		/*!
		 * \since v.5.2.3
		 * \brief Switch agent to special state and initiate stopping
		 * of SObjectizer Environment.
		 *
		 * Calls abort() if an exception is raised during work.
		 */
		void
		switch_agent_to_special_state_and_shutdown_sobjectizer(
			//! Agent who is the producer of the exception.
			so_5::rt::agent_t & a_exception_producer );

		/*!
		 * \since v.5.2.3
		 * \brief Switch agent to special state and deregister its cooperation.
		 *
		 * Calls abort() if an exception is raised during work.
		 */
		void
		switch_agent_to_special_state_and_deregister_coop(
			//! Agent who is the producer of the exception.
			so_5::rt::agent_t & a_exception_producer );

		//! Handle a bunch of demands.
		void
		serve_demands_block(
			//! Bunch of demands to be processed.
			demand_container_t & executed_demands );

	private:
		//! Demands queue.
		demand_queue_t m_queue;

		//! Thread status flag.
		enum
		{
			//! 0 - thread execution should be stopped.
			WORK_THREAD_STOP = 0,
			//! 1 - thread execution should be continued.
			WORK_THREAD_CONTINUE = 1
		};

		//! Thread status flag.
		/*!
		 * Available values are: WORK_THREAD_STOP, WORK_THREAD_CONTINUE
		 */
		std::atomic_long m_continue_work;

		//! Actual working thread.
		std::unique_ptr< std::thread > m_thread;

		/*!
		 * \since v.5.4.0
		 * \brief ID of working thread.
		 *
		 * \attention Receive the value only after start of thread body.
		 */
		so_5::current_thread_id_t m_thread_id;

		//! Owner of this working thread.
		/*!
		 * This reference is necessary to handle exceptions.
		 * The exception handler is got from the dispatcher.
		 */
		rt::dispatcher_t & m_disp;
};

/*!
 * \since v.5.4.0
 * \brief Shared pointer for work_thread.
 */
typedef std::shared_ptr< work_thread_t > work_thread_shptr_t;

} /* namespace work_thread */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

#endif

