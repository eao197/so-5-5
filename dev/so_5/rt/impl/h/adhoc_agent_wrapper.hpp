/*
	SObjectizer 5.
*/

/*!
 * \file
 * \since v.5.3.0
 * \brief Special agent class which would be used as wrapper for ad-hoc agents.
 */

#if !defined( SO_5__RT__IMPL__ADHOC_AGENT_WRAPPER_HPP )
#define SO_5__RT__IMPL__ADHOC_AGENT_WRAPPER_HPP

#include <so_5/rt/h/agent.hpp>

#include <functional>

namespace so_5
{

namespace rt
{

namespace impl
{

/*!
 * \since v.5.3.0
 * \brief Special agent class which would be used as wrapper for ad-hoc agents.
 *
 * \note The presence and usefulness of this class is based on the fact
 * that it is not necessary to subscribe agent exactly in so_define_agent()
 * method. The current implementation of agents and SObjectizer Run-Time
 * allows to subscribe and unsubscribe agent in any place and at any time.
 */
class SO_5_TYPE adhoc_agent_wrapper_t
	:	public agent_t
	{
	public :
		adhoc_agent_wrapper_t( so_environment_t & env );
		virtual ~adhoc_agent_wrapper_t();

		//! Set function for reaction on work start.
		void
		set_on_evt_start( std::function< void() > handler );

		//! Set function for reaction on work finish.
		void
		set_on_evt_finish( std::function< void() > handler );

		//! Set reaction for non-handled exceptions.
		void
		set_exception_reaction(
			exception_reaction_t reaction );

		virtual void
		so_evt_start();

		virtual void
		so_evt_finish();

		virtual exception_reaction_t
		so_exception_reaction() const;

	private :
		std::function< void() > m_on_start;
		std::function< void() > m_on_finish;
		exception_reaction_t m_exception_reaction;
	};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
