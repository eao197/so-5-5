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
#include <so_5/h/ret_code.hpp>
#include <so_5/h/types.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class agent_core_t;
class so_environment_impl_t;

} /* namespace impl */


class so_environment_t;
class agent_coop_t;

//! Typedef for agent_coop autopointer.
typedef std::unique_ptr< agent_coop_t > agent_coop_unique_ptr_t;

//! Agent cooperation.
/*!
 * The main purpose of cooperation is introduce several agents into
 * SObjectizer as a whole. A cooperation should be registered
 * (e.g. all agents from cooperations should be registered). If an agent from
 * cooperation cannot be registered then whole cooperation is not registered.
 *
 * Agents are added to cooperation by add_agent() method.
 *
 * After addition to cooperation the cooperation tooks care about
 * agent life-time.
 */
class SO_5_TYPE agent_coop_t
{
	private :
		friend class agent_t;
		friend class impl::agent_core_t;

		//! Constructor.
		agent_coop_t(
			//! Cooperation name.
			const nonempty_name_t & name,
			//! Default dispatcher binding.
			disp_binder_unique_ptr_t & coop_disp_binder,
			//! SObjectizer Environment.
			impl::so_environment_impl_t & env_impl );

	public:
		//! Create a cooperation.
		static agent_coop_unique_ptr_t
		create_coop(
			//! Cooperation name.
			const nonempty_name_t & name,
			//! Default dispatcher binding.
			disp_binder_unique_ptr_t & coop_disp_binder,
			//! SObjectizer Environment for which cooperation will be created.
			impl::so_environment_impl_t & env_impl );

		virtual ~agent_coop_t();

		//! Get cooperation name.
		const std::string &
		query_coop_name() const;

		//! Add agent to cooperation.
		/*!
		 * Cooperation tooks care about agent lifetime.
		 *
		 * Default dispatcher binding is used for agent.
		 */
		ret_code_t
		add_agent(
			const agent_ref_t & agent_ref );

		//! Add agent to cooperation with dispatcher binding.
		/*!
		 * Instead of default dispatcher binding the \a disp_binder
		 * is used for that agent during cooperation registration.
		 */
		ret_code_t
		add_agent(
			//! Agent.
			const agent_ref_t & agent_ref,
			//! Agent to dispatcher binder.
			disp_binder_unique_ptr_t disp_binder );

		//! Internal SObjectizer method.
		/*!
		 * Informs cooperation about full finishing of agent work.
		 */
		static inline void
		call_agent_finished( agent_coop_t & coop )
		{
			coop.agent_finished();
		}

		//! Internal SObjectizer method.
		/*!
		 * Initiate final deregistration stage.
		 */
		static inline void
		call_final_deregister_coop( agent_coop_t * coop )
		{
			coop->final_deregister_coop();
		}

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

		//! Typedef for agent information container.
		typedef std::vector< agent_with_disp_binder_t > agent_array_t;

		//! Bind agents to cooperation.
		void
		bind_agents_to_coop();

		//! Call define_agent method for all cooperation agents.
		void
		define_all_agents();

		//! Call undefine_agent method for all cooperation agents.
		void
		undefine_all_agents();

		//! Call undefine_agent method for the agent specified.
		/*!
		 * Call undefine_agent for all agents in range
		 * [m_agent_array.begin(), it).
		 */
		void
		undefine_some_agents(
			//! Right border of processing range.
			agent_array_t::iterator it );

		//! Bind agents to dispatcher.
		void
		bind_agents_to_disp();

		//! Unbind agent from dispatcher.
		/*!
		 * Unbinds all agents in range [m_agent_array.begin(), it).
		 */
		void
		unbind_agents_from_disp(
			//! Right border of processing range.
			agent_array_t::iterator it );

		//! Process signal about finished work of an agent.
		/*!
		 * Cooperation deregistration is a long process. All agents
		 * process events from their queues. When an agent detects that
		 * no more events in its queue it informs cooperation about this.
		 *
		 * When cooperation detects that all agents have finished their
		 * work it initiates agents destruction.
		 */
		void
		agent_finished();

		//! Do final deregistration stage.
		void
		final_deregister_coop();

		//! Cooperation name.
		const std::string m_coop_name;

		//! Object lock.
		ACE_Thread_Mutex & m_lock;

		//! Agent undefinition flag.
		bool m_agents_are_undefined;

		//! Default agent to dispatcher binder.
		disp_binder_ref_t m_coop_disp_binder;

		//! Cooperation agents.
		agent_array_t m_agent_array;

		//! SObjectizer Environment for which cooperation is created.
		impl::so_environment_impl_t & m_so_environment_impl;

		//! Count for working agents.
		atomic_counter_t m_working_agents_count;
};

//! Typedef for agent_coop smart pointer.
typedef std::shared_ptr< agent_coop_t > agent_coop_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
