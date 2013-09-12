/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A cooperation deregistration waiting queue definition.
*/

#if !defined( _SO_5__RT__IMPL__COOP_DEREG__DEREG_DEMAND_QUEUE_HPP_ )
#define _SO_5__RT__IMPL__COOP_DEREG__DEREG_DEMAND_QUEUE_HPP_

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include <so_5/rt/h/agent_coop.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

namespace coop_dereg
{

//
// dereg_demand_queue_t
//

//! A cooperation deregistration waiting queue.
/*!
 * Thread safe.
 *
 * Also stores shutdown flag.
 */
class dereg_demand_queue_t
{
	public:
		typedef std::vector< agent_coop_t* > dereg_demand_container_t;

		dereg_demand_queue_t();
		~dereg_demand_queue_t();

		//! Put cooperation into queue.
		void
		push( agent_coop_t * coop );

		//! Get cooperations from queue.
		/*!
		 * Will block if queue is empty.
		 *
		 * Return no cooperations if shutdown flag is set.
		 */
		void
		pop(
			/*! Demands receiver. */
			dereg_demand_container_t & demands );

		//! Initiate working.
		void
		start_service();

		//! Finish working.
		/*!
		 * Sets up shutdown flag.
		 */
		void
		stop_service();

	private:
		//! Waiting queue.
		dereg_demand_container_t m_demands;

		//! Object lock.
		ACE_Thread_Mutex m_lock;

		//! Condition variable for waking up sleeping thread.
		ACE_Condition_Thread_Mutex m_not_empty;

		//! Working status.
		/*!
		 * Value true means that queue is in working state.
		 * Value false serves as shutdown flag.
		 */
		bool m_in_service;
};

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
