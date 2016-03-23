/*
 * SObjectizer-5
 */

/*!
 * \file
 * \brief An implementation of thread pool dispatcher.
 * \since
 * v.5.4.0
 */

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <memory>
#include <map>
#include <iostream>
#include <atomic>

#include <so_5/rt/h/event_queue.hpp>
#include <so_5/rt/h/disp.hpp>

#include <so_5/disp/reuse/h/mpmc_ptr_queue.hpp>

#include <so_5/disp/thread_pool/impl/h/common_implementation.hpp>

namespace so_5
{

namespace disp
{

namespace thread_pool
{

namespace impl
{

using spinlock_t = so_5::default_spinlock_t;

class agent_queue_t;

//
// dispatcher_queue_t
//
using dispatcher_queue_t = so_5::disp::reuse::mpmc_ptr_queue_t< agent_queue_t >;

//
// agent_queue_t
//
/*!
 * \brief Event queue for the agent (or cooperation).
 * \since
 * v.5.4.0
 */
class agent_queue_t
	:	public event_queue_t
	,	private so_5::atomic_refcounted_t
	{
		friend class so_5::intrusive_ptr_t< agent_queue_t >;

	private :
		//! Actual demand in event queue.
		struct demand_t : public execution_demand_t
			{
				//! Next item in queue.
				demand_t * m_next;

				demand_t()
					:	m_next( nullptr )
					{}
				demand_t( execution_demand_t && original )
					:	execution_demand_t( std::move( original ) )
					,	m_next( nullptr )
					{}
			};

	public :
		//! Constructor.
		agent_queue_t(
			//! Dispatcher queue to work with.
			dispatcher_queue_t & disp_queue,
			//! Parameters for the queue.
			const params_t & params )
			:	m_disp_queue( disp_queue )
			,	m_max_demands_at_once( params.query_max_demands_at_once() )
			,	m_tail( &m_head )
			{}

		~agent_queue_t()
			{
				while( m_head.m_next )
					remove_head();
			}

		//! Push next demand to queue.
		virtual void
		push( execution_demand_t demand )
			{
				std::unique_ptr< demand_t > tail_demand{
						new demand_t( std::move( demand ) ) };

				bool was_empty;

				{
					std::lock_guard< spinlock_t > lock( m_lock );

					was_empty = (nullptr == m_head.m_next);

					m_tail->m_next = tail_demand.release();
					m_tail = m_tail->m_next;

					++m_size;
				}

				// Scheduling of the queue must be done when queue lock
				// is unlocked.
				if( was_empty )
					m_disp_queue.schedule( this );
			}

		//! Get the front demand from queue.
		/*!
		 * \attention This method must be called only on non-empty queue.
		 */
		execution_demand_t &
		front()
			{
				return *(m_head.m_next);
			}

		/*!
		 * \brief Queue emptyness indication.
		 *
		 * \since
		 * v.5.5.15.1
		 */
		enum class emptyness_t
			{
				empty,
				not_empty
			};

		/*!
		 * \brief Indication of possibility of continuation of demands processing.
		 *
		 * \since
		 * v.5.5.15.1
		 */
		enum class processing_continuation_t
			{
				//! Next demand can be processed.
				enabled,
				disabled
			};

		/*!
		 * \brief A result of erasing of the front demand from queue.
		 *
		 * \since
		 * v.5.5.15.1
		 */
		struct pop_result_t
			{
				//! Can demands processing be continued?
				processing_continuation_t m_continuation;
				//! Is event queue empty?
				emptyness_t m_emptyness;
			};

		//! Remove the front demand.
		/*!
		 * \note Return processing_continuation_t::disabled if
		 * \a demands_processed exceeds m_max_demands_at_once or if
		 * event queue is empty.
		 */
		pop_result_t
		pop(
			//! Count of consequently processed demands from that queue.
			std::size_t demands_processed )
			{
				// Actual deletion of old head must be performed
				// when m_lock will be released.
				std::unique_ptr< demand_t > old_head;
				{
					std::lock_guard< spinlock_t > lock( m_lock );

					old_head = remove_head();

					const auto emptyness = m_head.m_next ?
							emptyness_t::not_empty : emptyness_t::empty;

					if( emptyness_t::empty == emptyness )
						m_tail = &m_head;

					return pop_result_t{
							detect_continuation( emptyness, demands_processed ),
							emptyness };
				}
			}

		/*!
		 * \brief Wait while queue becomes empty.
		 *
		 * It is necessary because there is a possibility that
		 * after processing of demand_handler_on_finish cooperation
		 * will be destroyed and agents will be unbound from dispatcher
		 * before the return from demand_handler_on_finish.
		 *
		 * Without waiting for queue emptyness it could lead to
		 * dangling pointer to agent_queue in woring thread.
		 */
		void
		wait_for_emptyness()
			{
				bool empty = false;
				while( !empty )
					{
						{
							std::lock_guard< spinlock_t > lock( m_lock );
							empty = (nullptr == m_head.m_next);
						}

						if( !empty )
							std::this_thread::yield();
					}
			}

		/*!
		 * \brief Get the current size of the queue.
		 *
		 * \since
		 * v.5.5.4
		 */
		std::size_t
		size() const
			{
				return m_size.load( std::memory_order_acquire );
			}

	private :
		//! Dispatcher queue for scheduling processing of events from
		//! this queue.
		dispatcher_queue_t & m_disp_queue;

		//! Maximum count of demands to be processed consequently.
		const std::size_t m_max_demands_at_once;

		//! Object's lock.
		spinlock_t m_lock;

		//! Head of the demand's queue.
		/*!
		 * Never contains actual demand. Only m_next field is used.
		 */
		demand_t m_head;
		//! Tail of the demand's queue.
		/*!
		 * Must point to m_head if queue is empty or to the very
		 * last queue item otherwise.
		 */
		demand_t * m_tail;

		/*!
		 * \brief Current size of the queue.
		 * \since
		 * v.5.5.4
		 */
		std::atomic< std::size_t > m_size = { 0 };

		//! Helper method for deleting queue's head object.
		inline std::unique_ptr< demand_t >
		remove_head()
			{
				std::unique_ptr< demand_t > to_be_deleted{ m_head.m_next };
				m_head.m_next = m_head.m_next->m_next;

				--m_size;

				return to_be_deleted;
			}

		//! Can processing be continued?
		inline processing_continuation_t
		detect_continuation(
			emptyness_t emptyness,
			const std::size_t processed )
			{
				return emptyness_t::not_empty == emptyness &&
						processed < m_max_demands_at_once ? 
						processing_continuation_t::enabled :
						processing_continuation_t::disabled;
			}
	};

//
// work_thread_t
//
/*!
 * \brief Class of work thread for thread pool dispatcher.
 * \since
 * v.5.4.0
 */
class work_thread_t
	{
	public :
		//! Initializing constructor.
		work_thread_t( dispatcher_queue_t & queue )
			:	m_disp_queue( &queue )
			,	m_condition{ queue.allocate_condition() }
			{
			}

		void
		join()
			{
				m_thread.join();
			}

		//! Launch work thread.
		void
		start()
			{
				m_thread = std::thread( [this]() { body(); } );
			}

	private :
		//! Dispatcher's queue.
		dispatcher_queue_t * m_disp_queue;

		//! ID of thread.
		/*!
		 * Receives actual value inside body().
		 */
		so_5::current_thread_id_t m_thread_id;

		//! Actual thread.
		std::thread m_thread;

		//! Waiting object for long wait.
		so_5::disp::mpmc_queue_traits::condition_unique_ptr_t m_condition;

		//! Thread body method.
		void
		body()
			{
				m_thread_id = so_5::query_current_thread_id();

				agent_queue_t * agent_queue;
				while( nullptr !=
						(agent_queue = m_disp_queue->pop( *m_condition )) )
					{
						do_queue_processing( agent_queue );
					}
			}

		/*!
		 * \since
		 * v.5.5.15.1
		 *
		 * \brief Starts processing of demands from the queue specified.
		 *
		 * Starts from \a current_queue. Processes up to enabled number
		 * of events from that queue. Then if the queue is not empty
		 * tries to find another non-empty queue. If there is no such queue
		 * then continue processing of \a current_queue.
		 */
		void
		do_queue_processing( agent_queue_t * current_queue )
			{
				do
					{
						const auto e = process_queue( *current_queue );

						if( agent_queue_t::emptyness_t::not_empty == e )
							{
								// We can continue processing of that queue if
								// there is no more non-empty queues waiting.
								current_queue = m_disp_queue->try_switch_to_another(
										current_queue );
							}
						else
							// Handling of the current queue should be stopped.
							current_queue = nullptr;
					}
				while( current_queue != nullptr );
			}


		//! Processing of demands from agent queue.
		agent_queue_t::emptyness_t
		process_queue( agent_queue_t & queue )
			{
				std::size_t demands_processed = 0;
				agent_queue_t::pop_result_t pop_result;

				do
					{
						auto & d = queue.front();

						d.call_handler( m_thread_id );

						++demands_processed;
						pop_result = queue.pop( demands_processed );
					}
				while( agent_queue_t::processing_continuation_t::enabled ==
						pop_result.m_continuation );

				return pop_result.m_emptyness;
			}

	};

//
// adaptation_t
//
/*!
 * \brief Adaptation of common implementation of thread-pool-like dispatcher
 * to the specific of this thread-pool dispatcher.
 * \since
 * v.5.5.4
 */
struct adaptation_t
	{
		static const char *
		dispatcher_type_name()
			{
				return "tp"; // thread_pool.
			}

		static bool
		is_individual_fifo( const params_t & params )
			{
				return fifo_t::individual == params.query_fifo();
			}

		static void
		wait_for_queue_emptyness( agent_queue_t & queue )
			{
				queue.wait_for_emptyness();
			}
	};

//
// dispatcher_t
//
/*!
 * \brief Actual type of this thread-pool dispatcher.
 * \since
 * v.5.4.0
 */
using dispatcher_t =
		common_implementation::dispatcher_t<
				work_thread_t,
				dispatcher_queue_t,
				agent_queue_t,
				params_t,
				adaptation_t >;

} /* namespace impl */

} /* namespace thread_pool */

} /* namespace disp */

} /* namespace so_5 */

