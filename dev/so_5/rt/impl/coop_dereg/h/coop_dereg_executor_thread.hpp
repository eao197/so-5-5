/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition for dedicated thread for cooperation deregistration.
*/

#if !defined( _SO_5__RT__IMPL__COOP_DEREG__COOP_DEREG_EXECUTOR_THREAD_HPP_ )
#define _SO_5__RT__IMPL__COOP_DEREG__COOP_DEREG_EXECUTOR_THREAD_HPP_

#include <so_5/rt/impl/coop_dereg/h/dereg_demand_queue.hpp>

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
// coop_dereg_executor_thread_t
//

//! A dedicated for cooperation deregistration thread.
/*!
 * This thread is used for doing actions of cooperation deregistration.
 * To execute those actions a cooperation is put itsefl to special
 * deregistration queue. This thread handles that queue.
 */
class coop_dereg_executor_thread_t
{
	public:
		coop_dereg_executor_thread_t();
		~coop_dereg_executor_thread_t();

		//! Launch thread.
		void
		start();

		//! Send a shutdown signal to thread.
		void
		shutdown();

		//! Wait for thread shutdown.
		void
		wait();

		//! Push cooperation to waiting queue.
		void
		push_dereg_demand(
			agent_coop_t * coop );

	protected:
		//! Thread body.
		void
		body();

		//! Thread entry point for ACE_Thread_Manager.
		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

	private:
		//! Waiting queue.
		dereg_demand_queue_t m_dereg_demand_queue;

		//! This thread id.
		/*!
			\note Has actual value only after start().
		*/
		ACE_thread_t m_tid;
};

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
