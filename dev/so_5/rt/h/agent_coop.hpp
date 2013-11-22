/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Agent cooperation definition.
*/

#if !defined( _SO_5__RT__AGENT_COOP_HPP_ )
#define _SO_5__RT__AGENT_COOP_HPP_

#include <vector>
#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/h/exception.hpp>
#include <so_5/h/types.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class agent_core_t;
class agent_coop_private_iface_t;
class so_environment_impl_t;

} /* namespace impl */


class so_environment_t;
class agent_coop_t;

//! Typedef for the agent_coop autopointer.
typedef std::unique_ptr< agent_coop_t > agent_coop_unique_ptr_t;

//! Agent cooperation.
/*!
 * The main purpose of the cooperation is the introducing of several agents into
 * SObjectizer as a single unit. A cooperation should be registered.
 *
 * For the cooperation to be successfuly registered all of its agents must 
 * successfuly pass registration steps (so-define, bind to the dispatcher). 
 * If at least one agent out of this cooperation fails to pass any of 
 * mentioned steps, the cooperation will not be registered and 
 * all of agents will run procedures opposite to registration 
 * steps (unbind from the dispatcher, so-undefine) which had been successfuly 
 * taken for the particulary agent in the reverse order.
 *
 * Agents are added to the cooperation by the add_agent() method.
 *
 * After addition to the cooperation the cooperation takes care about
 * the agent lifetime.
 */
class SO_5_TYPE agent_coop_t
{
	private :
		friend class agent_t;
		friend class impl::agent_core_t;
		friend class impl::agent_coop_private_iface_t;

		//! Constructor.
		agent_coop_t(
			//! Cooperation name.
			const nonempty_name_t & name,
			//! Default dispatcher binding.
			disp_binder_unique_ptr_t coop_disp_binder,
			//! SObjectizer Environment.
			impl::so_environment_impl_t & env_impl );

	public:
		//! Create a cooperation.
		static agent_coop_unique_ptr_t
		create_coop(
			//! Cooperation name.
			const nonempty_name_t & name,
			//! Default dispatcher binding.
			disp_binder_unique_ptr_t coop_disp_binder,
			//! SObjectizer Environment for which cooperation will be created.
			impl::so_environment_impl_t & env_impl );

		virtual ~agent_coop_t();

		//! Get cooperation name.
		const std::string &
		query_coop_name() const;

		//! Add agent to cooperation.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		template< class AGENT >
		inline void
		add_agent(
			//! Agent.
			std::unique_ptr< AGENT > agent )
		{
			this->do_add_agent( agent_ref_t( agent.release() ) );
		}

		//! Add agent to cooperation via raw pointer.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		inline void
		add_agent(
			//! Agent.
			agent_t * agent )
		{
			this->do_add_agent( agent_ref_t( agent ) );
		}

		//! Add agent to the cooperation with the dispatcher binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		template< class AGENT >
		inline void
		add_agent(
			//! Agent.
			std::unique_ptr< AGENT > agent,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder )
		{
			this->do_add_agent(
				agent_ref_t( agent.release() ),
				std::move(disp_binder) );
		}

		//! Add agent to the cooperation via raw pointer and with the dispatcher
		//! binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		inline void
		add_agent(
			//! Agent.
			agent_t * agent,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder )
		{
			this->do_add_agent(
				agent_ref_t( agent ),
				std::move(disp_binder) );
		}

		//! Internal SObjectizer method.
		/*!
		 * Informs cooperation about full finishing of agent's or
		 * child cooperation work.
		 */
		static inline void
		call_entity_finished( agent_coop_t & coop )
		{
			coop.entity_finished();
		}

		//! Internal SObjectizer method.
		/*!
		 * Initiate a final deregistration stage.
		 */
		static inline void
		call_final_deregister_coop( agent_coop_t * coop )
		{
			coop->final_deregister_coop();
		}

		/*!
		 * \name Methods for working with name of parent cooperation.
		 * \{
		 */
		/*!
		 * \since v.5.2.3
		 * \brief Does cooperation have parent cooperation?
		 */
		bool
		has_parent_coop() const;

		/*!
		 * \since v.5.2.3
		 * \brief Set name of the parent cooperation.
		 */
		void
		set_parent_coop_name(
			const nonempty_name_t & name );

		/*!
		 * \since v.5.2.3
		 * \brief Get name of the parent cooperation.
		 *
		 * \throw exception_t if the parent cooperation name is not set.
		 */
		const std::string &
		parent_coop_name() const;
		/*!
		 * \}
		 */

	private:
		//! Information about agent and its dispatcher binding.
		struct agent_with_disp_binder_t
		{
			agent_with_disp_binder_t(
				const agent_ref_t & agent_ref,
				const disp_binder_ref_t & binder )
				:
					m_agent_ref( agent_ref ),
					m_binder( binder )
			{}

			//! Agent.
			agent_ref_t m_agent_ref;

			//! Agent to dispatcher binder.
			disp_binder_ref_t m_binder;
		};

		//! Typedef for the agent information container.
		typedef std::vector< agent_with_disp_binder_t > agent_array_t;

		//! Cooperation name.
		const std::string m_coop_name;

		//! Default agent to the dispatcher binder.
		disp_binder_ref_t m_coop_disp_binder;

		//! Cooperation agents.
		agent_array_t m_agent_array;

		//! SObjectizer Environment for which cooperation is created.
		impl::so_environment_impl_t & m_so_environment_impl;

		//! Count for entities.
		/*!
		 * Since v.5.2.3 this counter includes:
		 * - count of agents from cooperation;
		 * - count of direct child cooperations;
		 */
		atomic_counter_t m_working_entities_count;

		/*!
		 * \since v.5.2.3.
		 * \brief Name of the parent cooperation.
		 *
		 * Empty value means than there is no parent cooperation.
		 */
		std::string m_parent_coop_name;

		/*!
		 * \since v.5.2.3.
		 * \brief Pointer to parent cooperation.
		 *
		 * Gets the value only if there is the parent cooperation and
		 * cooperation itself is registered successfully.
		 */
		agent_coop_t * m_parent_coop_ptr;

		//! Add agent to cooperation.
		/*!
		 * Cooperation takes care about agent lifetime.
		 *
		 * Default dispatcher binding is used for the agent.
		 */
		void
		do_add_agent(
			const agent_ref_t & agent_ref );

		//! Add agent to the cooperation with the dispatcher binding.
		/*!
		 * Instead of the default dispatcher binding the \a disp_binder
		 * is used for this agent during the cooperation registration.
		 */
		void
		do_add_agent(
			//! Agent.
			const agent_ref_t & agent_ref,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder );

		/*!
		 * \since v.5.2.3
		 * \brief Perform all neccessary actions related to
		 * cooperation registration.
		 */
		void
		do_registration_specific_actions(
			//! Pointer to the parent cooperation.
			//! Contains nullptr if there is no parent cooperation.
			agent_coop_t * agent_coop );

		//! Bind agents to the cooperation.
		void
		bind_agents_to_coop();

		//! Calls define_agent method for all cooperation agents.
		void
		define_all_agents();

		//! Calls undefine_agent method for all cooperation agents.
		void
		undefine_all_agents();

		//! Calls undefine_agent method for the agent specified.
		/*!
		 * Calls undefine_agent for all agents in range
		 * [m_agent_array.begin(), it).
		 */
		void
		undefine_some_agents(
			//! Right border of the processing range.
			agent_array_t::iterator it );

		//! Bind agents to the dispatcher.
		void
		bind_agents_to_disp();

		//! Unbind agent from the dispatcher.
		/*!
		 * Unbinds all agents in range [m_agent_array.begin(), it).
		 */
		void
		unbind_agents_from_disp(
			//! Right border of the processing range.
			agent_array_t::iterator it );

		//! Process signal about finished work of an agent or
		//! child cooperation.
		/*!
		 * Cooperation deregistration is a long process. All agents
		 * process events out of their queues. When an agent detects that
		 * no more events in its queue it informs the cooperation about this.
		 *
		 * When cooperation detects that all agents have finished their
		 * work it initiates the agent's destruction.
		 *
		 * Since v.5.2.3 this method used not only for agents of cooperation
		 * but and for children cooperations. Because final step of
		 * cooperation deregistration could be initiated only when all
		 * children cooperations are deregistered and destroyed.
		 */
		void
		entity_finished();

		//! Do the final deregistration stage.
		void
		final_deregister_coop();

		/*!
		 * \since v.5.2.3
		 * \brief Get pointer to the parent cooperation.
		 *
		 * \retval nullptr if there is no parent cooperation.
		 */
		agent_coop_t *
		parent_coop_ptr() const;
};

//! Typedef for the agent_coop smart pointer.
typedef std::shared_ptr< agent_coop_t > agent_coop_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
