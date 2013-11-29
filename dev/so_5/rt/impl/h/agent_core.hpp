/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A class for a part of the agent/environment functionality.
*/

#if !defined( _SO_5__RT__IMPL__AGENT_CORE_HPP_ )
#define _SO_5__RT__IMPL__AGENT_CORE_HPP_

#include <map>
#include <memory>
#include <set>
#include <string>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include <so_5/h/exception.hpp>

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

namespace agent_core_details
{

class deregistration_processor_t;

} /* namespace agent_core_details */

//
// agent_coop_private_iface_t
//
/*!
 * \since v.5.2.3
 * \brief A special class for accessing private members of agent_coop.
 */
class agent_coop_private_iface_t
{
	public :
		inline static void
		undefine_all_agents( agent_coop_t & coop )
		{
			coop.undefine_all_agents();
		}

		inline static agent_coop_t *
		parent_coop_ptr( const agent_coop_t & coop )
		{
			return coop.parent_coop_ptr();
		}

		inline static coop_notificators_container_ref_t
		reg_notificators( const agent_coop_t & coop )
		{
			return coop.reg_notificators();
		}

		inline static coop_notificators_container_ref_t
		dereg_notificators( const agent_coop_t & coop )
		{
			return coop.dereg_notificators();
		}
};

//
// agent_core_t
//

//! A class for a part of the agent/environment functionality.
class agent_core_t
{
		agent_core_t( const agent_core_t & );
		void
		operator = ( const agent_core_t & );

		friend class so_5::rt::impl::agent_core_details::
				deregistration_processor_t;

	public:
		explicit agent_core_t(
			//! SObjectizer Environment.
			so_environment_t & so_environment_impl,
			//! Mutex pool size for agent queues.
			unsigned int agent_queue_mutex_pool_size,
			//! Cooperation action listener.
			coop_listener_unique_ptr_t coop_listener );

		~agent_core_t();

		//! Do initialization.
		void
		start();

		//! Finish work.
		/*!
		 * Initiates deregistration of all agents. Waits for complete
		 * deregistration for all of them. Waits for termination of
		 * cooperation deregistration thread.
		 */
		void
		finish();

		//! Create a local event queue for an agent.
		local_event_queue_unique_ptr_t
		create_local_queue();

		//! Register cooperation.
		void
		register_coop(
			//! Cooperation to be registered.
			agent_coop_unique_ptr_t agent_coop );

		//! Deregister cooperation.
		void
		deregister_coop(
			//! Cooperation name which being deregistered.
			const nonempty_name_t & name );

		//! Notification about readiness of the cooperation deregistration.
		void
		ready_to_deregister_notify(
			agent_coop_t * coop );

		//! Do final actions of the cooperation deregistration.
		void
		final_deregister_coop(
			//! Cooperation name to be deregistered.
			/*!
			 * \note Cooperation name must be passed by value because
			 * reference can become invalid during work of this method.
			*/
			const std::string coop_name );

		//! Initiate start of the cooperation deregistration.
		void
		start_deregistration();

		//! Wait for a signal about start of the cooperation deregistration.
		void
		wait_for_start_deregistration();

		//! Deregisted all cooperations.
		/*!
		 * All cooperations will be deregistered at the SObjectizer shutdown.
		*/
		void
		deregister_all_coop();

		//! Wait for end of all cooperations deregistration.
		void
		wait_all_coop_to_deregister();

	private:
		//! Typedef for map from cooperation name to the cooperation.
		typedef std::map<
				std::string,
				agent_coop_ref_t >
			coop_map_t;

		/*!
		 * \since v.5.2.3
		 * \brief Typedef for pair of names of parent and child cooperations.
		 *
		 * \a first -- name of parent.
		 * \a second -- name of child.
		 */
		typedef std::pair< std::string, std::string >
			parent_child_coop_names_t;

		/*!
		 * \since v.5.2.3
		 * \brief Typedef for set of parent-child names pairs.
		 */
		typedef std::set< parent_child_coop_names_t >
			parent_child_coop_relation_t;

		//! SObjectizer Environment to work with.
		so_environment_t & m_so_environment;

		//! Mutex pool for agent's event queues.
		util::mutex_pool_t< ACE_Thread_Mutex > m_agent_queue_mutex_pool;

		//! Lock for operations on cooperations.
		ACE_Thread_Mutex m_coop_operations_lock;

		//! Condition variable for the deregistration start indication.
		ACE_Condition_Thread_Mutex m_deregistration_started_cond;

		//! Condition variable for the deregistration finish indication.
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

		/*!
		 * \since v.5.2.3
		 * \brief Information about parent and child cooperations
		 * relationship.
		 */
		parent_child_coop_relation_t m_parent_child_relations;

		//! An auxiliary method for the std::for_each.
		static void
		coop_undefine_all_agents( agent_core_t::coop_map_t::value_type & coop );

		/*!
		 * \since v.5.2.3
		 * \brief Ensures that name of new cooperation is unique.
		 */
		void
		ensure_new_coop_name_unique(
			const std::string & coop_name ) const;

		/*!
		 * \since v.5.2.3
		 * \brief Checks that parent cooperation is registered if its name
		 * is set for the cooperation specified.
		 *
		 * \retval nullptr if no parent cooperation name set. Otherwise the
		 * pointer to parent cooperation is returned.
		 */
		agent_coop_t *
		find_parent_coop_if_necessary(
			const agent_coop_t & coop_to_be_registered ) const;

		/*!
		 * \since v.5.2.3
		 * \brief Next step of cooperation registration.
		 *
		 * Initiate cooperation registration actions and
		 * store cooperation info in registered cooperations map.
		 */
		void
		next_coop_reg_step__update_registered_coop_map(
			//! Cooperation to be registered.
			const agent_coop_ref_t & coop_ref,
			//! Pointer to parent cooperation.
			//! Equal to nullptr if \a coop has no parent.
			agent_coop_t * parent_coop_ptr );

		/*!
		 * \since v.5.2.3
		 * \brief Next step of cooperation registration.
		 *
		 * Updates information about parent-child cooperation relationship
		 * and goes further.
		 */
		void
		next_coop_reg_step__parent_child_relation(
			//! Cooperation to be registered.
			const agent_coop_ref_t & coop,
			//! Pointer to parent cooperation.
			//! Equal to nullptr if \a coop has no parent.
			agent_coop_t * parent_coop_ptr );

		/*!
		 * \since v.5.2.3
		 * \brief Do final action for cooperation deregistration.
		 *
		 * If parent cooperation exists then parent-child relation
		 * is handled appropriatelly.
		 *
		 * Information about cooperation is removed from m_deregistered_coop.
		 */
		coop_notificators_container_ref_t
		finaly_remove_cooperation_info(
			const std::string & coop_name );

		/*!
		 * \since v.5.2.3
		 * \brief Do all job related to sending notification about
		 * cooperation registration.
		 */
		void
		do_coop_reg_notification_if_necessary(
			const std::string & coop_name,
			const coop_notificators_container_ref_t & notificators ) const;

		/*!
		 * \since v.5.2.3
		 * \brief Do all job related to sending notification about
		 * cooperation deregistration.
		 */
		void
		do_coop_dereg_notification_if_necessary(
			const std::string & coop_name,
			const coop_notificators_container_ref_t & notificators ) const;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
