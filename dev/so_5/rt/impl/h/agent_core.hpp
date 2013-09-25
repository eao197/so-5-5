/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A class for a part of agent/environment functionality.
*/

#if !defined( _SO_5__RT__IMPL__AGENT_CORE_HPP_ )
#define _SO_5__RT__IMPL__AGENT_CORE_HPP_

#include <memory>
#include <string>
#include <map>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/util/h/mutex_pool.hpp>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/coop_listener.hpp>

#include <so_5/rt/impl/h/local_event_queue.hpp>

#include <so_5/rt/impl/coop_dereg/h/coop_dereg_executor_thread.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// agent_core_t
//

//! A class for a part of agent/environment functionality.
class agent_core_t
{
		agent_core_t( const agent_core_t & );
		void
		operator = ( const agent_core_t & );

	public:
		explicit agent_core_t(
			//! SObjectizer environment.
			so_environment_t & so_environment_impl,
			//! Size of mutex pool for cooperations.
			unsigned int agent_coop_mutex_pool_size,
			//! Size of mutex pool for agent queues.
			unsigned int agent_queue_mutex_pool_size,
			//! Cooperation action listener.
			coop_listener_unique_ptr_t coop_listener );

		~agent_core_t();

		//! Do initialization.
		void
		start();

		//! Send a shutdown signal.
		void
		shutdown();

		//! Blocking wait for complete shutdown.
		void
		wait();

		/*!
		 * \name Methods for working with cooperation mutexes.
		 * \{
		 */
		//! Get a cooperation mutex.
		ACE_Thread_Mutex &
		allocate_agent_coop_mutex();

		//! Return cooperation mutex.
		void
		deallocate_agent_coop_mutex(
			ACE_Thread_Mutex & m );
		/*!
		 * \}
		 */

		//! Create a local event queue for an agent.
		local_event_queue_unique_ptr_t
		create_local_queue();

		//! Register cooperation.
		ret_code_t
		register_coop(
			//! Cooperation to be registered.
			agent_coop_unique_ptr_t agent_coop,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Deregister cooperation.
		ret_code_t
		deregister_coop(
			//! Name of cooperation being deregistered.
			const nonempty_name_t & name,
			//! Exception strategy.
			throwing_strategy_t throwing_strategy );

		//! Notification about readiness of cooperation deregistration.
		void
		ready_to_deregister_notify(
			agent_coop_t * coop );

		//! Do final actions of cooperation deregistration.
		void
		final_deregister_coop(
			//! Name of cooperation to be deregistered.
			/*!
			 * Name of cooperation should be passed by value because
			 * reference could become invalid during work of this method.
			*/
			const std::string coop_name );

		//! Initiate start of cooperation deregistration.
		void
		start_deregistration();

		//! Wait for signal about start of cooperation deregistration.
		void
		wait_for_start_deregistration();

		//! Deregisted all cooperations.
		/*!
		 * All cooperations should be deregistered at SObjectize shutdown.
		*/
		void
		deregister_all_coop();

		//! Wait for end of all cooperations deregistration.
		void
		wait_all_coop_to_deregister();

	private:
		//! Typedef for map from cooperation name to cooperation.
		typedef std::map<
				std::string,
				agent_coop_ref_t >
			coop_map_t;

		//! An auxiliary method for std::for_each.
		static void
		coop_undefine_all_agents( agent_core_t::coop_map_t::value_type & coop );

		//! SObjectizer Environment to work with.
		so_environment_t & m_so_environment;

		//! Mutex pool for cooperations.
		util::mutex_pool_t< ACE_Thread_Mutex > m_agent_coop_mutex_pool;

		//! Mutex pool for agent's event queues.
		util::mutex_pool_t< ACE_Thread_Mutex > m_agent_queue_mutex_pool;

		//! Lock for operations on cooperations.
		ACE_Thread_Mutex m_coop_operations_lock;

		//! Condition variable for deregistration start indication.
		ACE_Condition_Thread_Mutex m_deregistration_started_cond;

		//! Condition variable for deregistration finish indication.
		ACE_Condition_Thread_Mutex m_deregistration_finished_cond;

		//! Indicator for all cooperation deregistration.
		bool m_deregistration_started;

		//! Map of registered cooperations.
		coop_map_t m_registered_coop;

		//! Map of cooperations being deregistered.
		coop_map_t m_deregistered_coop;

		//! Cooperation deregistration thread.
		coop_dereg::coop_dereg_executor_thread_t m_coop_dereg_executor;

		//! Cooperation actions listener.
		coop_listener_unique_ptr_t m_coop_listener;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
