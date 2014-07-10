/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Working thread for dispatchers.
*/

#if !defined( _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_ )
#define _SO_5__DISP__REUSE__WORK_THREAD__WORK_THREAD_HPP_

#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/event_queue.hpp>

namespace so_5
{

namespace disp
{

namespace reuse
{

namespace work_thread
{

//
// demand_t
//

//! Element of queue of demands to process agent events.
struct demand_t
{
	//! Receiver of demand.
	so_5::rt::agent_t * m_receiver;
	//! Event handler.
	so_5::rt::event_caller_block_ref_t m_event_caller_block;
	//! Event incident.
	so_5::rt::message_ref_t m_message_ref;
	//! Demand handler.
	so_5::rt::demand_handler_pfn_t m_demand_handler;

	demand_t(
		so_5::rt::agent_t * receiver,
		so_5::rt::event_caller_block_ref_t event_caller_block,
		so_5::rt::message_ref_t message_ref,
		so_5::rt::demand_handler_pfn_t demand_handler )
		:	m_receiver( receiver )
		,	m_event_caller_block( std::move( event_caller_block ) )
		,	m_message_ref( std::move( message_ref ) )
		,	m_demand_handler( demand_handler )
		{}
};

//! Typedef for demand's container.
typedef std::deque< demand_t > demand_container_t;

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
			so_5::rt::agent_t * receiver,
			const so_5::rt::event_caller_block_ref_t & event_caller_block,
			const so_5::rt::message_ref_t & message_ref,
			so_5::rt::demand_handler_pfn_t demand_handler );
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
		std::mutex m_lock;
		std::condition_variable m_not_empty;
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

