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

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/agent.hpp>

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
	//! Agent which events will be dispatched.
	so_5::rt::agent_t * m_agent_ptr;

	//! Count of events to dispatch.
	unsigned int m_event_cnt;

	demand_t()
		:	m_agent_ptr( nullptr )
		,	m_event_cnt( 0 )
	{}

	demand_t(
		//! Agent which events will be dispatched.
		so_5::rt::agent_t * agent_ptr,
		//! Count of events to dispatch.
		unsigned int event_cnt )
		:	m_agent_ptr( agent_ptr )
		,	m_event_cnt( event_cnt )
	{}
};

//! Typedef for demands of the queue container.
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
class demand_queue_t
{
	public:
		demand_queue_t();
		~demand_queue_t();

		//! Put demand into the queue.
		void
		push(
			//! Agent which events will be dispatched.
			so_5::rt::agent_t * agent_ptr,
			//! Count of events to dispatch.
			unsigned int event_cnt );

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
		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_not_empty;
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

		//! Shedule event(s) for the agent.
		/*!
		 * \note In most cases event_count is equal to 1.
		 * The only case when \a event_count can be greater than 1 is 
		 * when disp_binder switches from the void-dispatcher to the 
		 * real-dispatcher.
		 * \see g_void_dispatcher.
		 * \see agent_t::bind_to_disp().
		*/
		void
		put_event_execution_request(
			//! Events will be sheduled to this agent.
			so_5::rt::agent_t * agent_ptr,
			//! Count of events to be scheduled.
			unsigned int event_count );

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

		//! Thread entry point for the ACE_Thread_Manager.
		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

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

		//! Thread identifier for this object.
		/*!
			\note m_tid has the actual value only after start().
		*/
		ACE_thread_t m_tid;

		//! Owner of this working thread.
		/*!
		 * This reference is necessary to handle exceptions.
		 * The exception handler is got from the dispatcher.
		 */
		rt::dispatcher_t & m_disp;
};

} /* namespace work_thread */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

#endif

