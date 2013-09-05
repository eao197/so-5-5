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

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Atomic_Op.h>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/agent_ref.hpp>

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
	//! Agent which events should be dispatched.
	so_5::rt::agent_ref_t m_agent_ref;

	//! Count of events to dispatch.
	unsigned int m_event_cnt;

	demand_t()
		:
			m_event_cnt( 0 )
	{}

	demand_t(
		//! Agent which events should be dispatched.
		const so_5::rt::agent_ref_t & agent_ref,
		//! Count of events to dispatch.
		unsigned int event_cnt )
		:
			m_agent_ref( agent_ref ),
			m_event_cnt( event_cnt )
	{}
};

//! Typedef for demands queue container.
typedef std::deque< demand_t > demand_container_t;

//
// demand_queue_t
//

//! Queue of demands to process agent events.
/*!
	Have shutdown flag inside.

	Thread safe and indendent to use by several concurrent threads.
*/
class demand_queue_t
{
	public:
		demand_queue_t();
		~demand_queue_t();

		//! Put demand into queue.
		void
		push(
			//! Agent which events should be dispatched.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Count of events to dispatch.
			unsigned int event_cnt );

		enum
		{
			//! Demand has been extracted
			demand_extracted = 1,
			//! Demand has not been extracted because of shutdown.
			shutting_down = 2,
			//! Demand has not been extracted because demand queue is empty.
			no_demands = 3
		};

		//! Try to extract demands from queue.
		/*!
			If there is no demands in queue then current thread
			will sleep until:
			- demand put in queue;
			- shutdown signal.
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

		//! \name Objects for thread safety.
		//! \{
		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_not_empty;
		//! \}

		//! Service flag.
		/*!
			true -- should do the service, methods push/pop should work.
			false -- service should be stopped.
		*/
		bool m_in_service;
};

//
// work_thread_t
//

//! Working thread.
/*!
 * Working thread should be used inside some dispatcher.
 * And life time of dispatcher object should be longer than
 * life time of working thread object.
 */
class work_thread_t
{
	public:
		work_thread_t(
			rt::dispatcher_t & disp );

		~work_thread_t();

		//! Shedule event for agent.
		void
		put_event_execution_request(
			//! Agent for which events should be sheduled.
			const so_5::rt::agent_ref_t & agent_ref,
			//! Count of events to be scheduled.
			unsigned int event_count );

		//! Start working thread.
		void
		start();

		//! Send shutdown signal to working thread.
		void
		shutdown();

		//! Wait full stop of working thread.
		/*!
		 * All non-processed demands from queue will be destroyed
		 * after stop of working thread.
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
			const so_5::rt::agent_ref_t & a_exception_producer );

		//! Handle a bunch of demands.
		void
		serve_demands_block(
			//! Bunch of demands to be processed.
			demand_container_t & executed_demands );

		//! Thread entry point for ACE_Thread_Manager.
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
		ACE_Atomic_Op< ACE_Thread_Mutex, long > m_continue_work;

		//! Thread identifier for that object.
		/*!
			\note Has actual value only after start().
		*/
		ACE_thread_t m_tid;

		//! Owner of that working thread.
		/*!
		 * This reference is necessary to handle exceptions.
		 * The exception handler is get from dispatcher.
		 */
		rt::dispatcher_t & m_disp;
};

} /* namespace work_thread */

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

#endif

