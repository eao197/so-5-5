/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.8
 * \file
 * \brief A demand queue for dispatcher with one common working
 * thread and support of demands priority.
 */

#pragma once

#include <memory>

#include <so_5/rt/h/execution_demand.hpp>

#include <so_5/disp/prio/h/priority.hpp>

#include <so_5/disp/reuse/locks/h/locks.hpp>

namespace so_5 {

namespace disp {

namespace prio {

namespace common_thread {

namespace impl {

//
// demand_t
//
/*!
 * \since v.5.5.8
 * \brief A single execution demand.
 */
struct demand_t : public so_5::rt::execution_demand_t
	{
		//! Next demand in the queue.
		demand_t * m_next = nullptr;

		//! Initializing constructor.
		demand_t( so_5::rt::execution_demand_t && source )
			:	so_5::rt::execution_demand_t( std::move( source ) )
			{}
	};

//
// demand_unique_ptr_t
//
/*!
 * \since v.5.5.8
 * \brief An alias for unique_ptr to demand.
 */
using demand_unique_ptr_t = std::unique_ptr< demand_t >;

//
// demand_queue_t
//
/*!
 * \since v.5.5.8
 * \brief A demand queue with support of demands priorities.
 */
class demand_queue_t
	{
		//! Description of queue for one priority.
		struct priority_queue_info_t
			{
				//! Head of the queue.
				/*! Null if queue is empty. */
				demand_t * m_head = nullptr;
				//! Tail of the queue.
				/*! Null if queue is empty. */
				demand_t * m_tail = nullptr;
			};

	public :
		//! This exception is thrown when pop is called after stop.
		class shutdown_ex_t : public std::runtime_error
			{};

		demand_queue_t()
			{}
		~demand_queue_t()
			{
				for( auto & q : m_priorities )
					cleanup_queue( q );
			}

		//! Set the shutdown signal.
		void
		stop()
			{
				so_5::disp::reuse::locks::combined_queue_lock_guard_t lock{ m_lock };

				m_shutdown = true;

				if( !m_current_priority )
					// There could be a sleeping working thread.
					// It must be notified.
					lock.notify_one();
			}

		//! Push a new demand to the queue.
		void
		push(
			//! Priority for the demand.
			so_5::disp::prio::priority_t priority,
			//! Demand to be pushed.
			execution_demand_t && demand )
			{
				// Create a new demand before queue object will be locked.
				demand_unique_ptr_t new_demand{ new demand_t{ std::move(demand) } };

				so_5::disp::reuse::locks::combined_queue_lock_guard_t lock{ m_lock };

				auto & queue_to_push = queue_by_priority( priority );
				add_demand_to_queue( queue_to_push, std::move( new_demand ) );

				if( !m_current_priority )
					{
						// Queue was empty. A sleeping working thread must
						// be notified.
						m_current_priority = &queue_to_push;
						lock.notify_one();
					}
				else if( m_current_priority < &queue_to_push )
					// New demand has greater priority than the previous.
					m_current_priority = &queue_to_push;
			}

		//! Pop demand from the queue.
		/*!
		 * \throw shutdown_ex_t in the case when queue is shut down.
		 */
		demand_unique_ptr_t
		pop()
			{
				so_5::disp::reuse::locks::combined_queue_unique_lock_t lock{ m_lock };

				while( !m_shutdown && !m_current_priority )
					lock.wait_for_notify();

				if( m_shutdown )
					throw shutdown_ex_t();

				demand_unique_ptr_t result{ m_current_priority->m_head };

				m_current_priority->m_head = result->m_next;
				result->m_next = nullptr;

				if( !m_current_priority->m_head )
					{
						// Queue become empty.
						m_current_priority->m_tail = nullptr;

						// A non-empty subqueue with lower priority needs to be found.
						while( m_current_priority > m_priorities )
							{
								--m_current_priority;
								if( m_current_priority->m_head )
									return result;
							}

						m_current_priority = nullptr;
					}

				return result;
			}

	private :
		//! Destroy all demands in the queue specified.
		void
		cleanup_queue( priority_queue_info_t & queue_info )
			{
				auto h = queue_info.m_head;
				while( h )
					{
						demand_unique_ptr_t t{ h };
						h = h->m_next;
					}
			};

		//! Get queue for the priority specified.
		priority_queue_info_t &
		queue_by_priority( so_5::disp::prio::priority_t priority )
			{
				return m_priorities[ priority ];
			}

		//! Add a new demand to the tail of the queue specified.
		void
		add_demand_to_queue(
			priority_queue_info_t & queue,
			demand_unique_ptr_t demand )
			{
				if( queue.m_tail )
					// Queue is not empty. Tail will be modified.
					queue.m_tail->m_next = demand.release();
				else
					{
						// Queue is empty. The whole description will be modified.
						queue.m_head = demand.release();
						queue.m_tail = queue.m_head;
					}
			}
	};

} /* namespace impl */

} /* namespace common_thread */

} /* namespace prio */

} /* namespace disp */

} /* namespace so_5 */

