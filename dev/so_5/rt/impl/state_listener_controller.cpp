/*
	SObjectizer 5.
*/

#include <algorithm>

#include <so_5/rt/impl/h/state_listener_controller.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

namespace /* ananymous */
{

//
// destroyable_listener_item_t
//

//! Ёлемент списка слушателей,
//! котрый не уничтожает подконтрольного слушател€.
class destroyable_listener_item_t
	:
		public agent_state_listener_t
{
	public:
		destroyable_listener_item_t(
			agent_state_listener_unique_ptr_t state_listener )
			:
				m_state_listener( std::move( state_listener ) )
		{}

		virtual ~destroyable_listener_item_t()
		{}

		//! ¬ызываетс€ после успешной смены состо€ни€ агента.
		virtual void
		changed(
			//! јгент, чье состо€ние изменилось.
			agent_t & agent,
			//! “екущего состо€ни€ агента.
			const state_t & state )
		{
			m_state_listener->changed( agent, state );
		}

	private:
		agent_state_listener_unique_ptr_t m_state_listener;
};

//
// nondestroyable_listener_item_t
//

//! Ёлемент списка слушателей,
//! котрый не уничтожает подконтрольного слушател€.
class nondestroyable_listener_item_t
	:
		public agent_state_listener_t
{
	public:
		nondestroyable_listener_item_t(
			agent_state_listener_t & state_listener )
			:
				m_state_listener( state_listener )
		{}

		virtual ~nondestroyable_listener_item_t()
		{}

		//! ¬ызываетс€ после успешной смены состо€ни€ агента.
		virtual void
		changed(
			//! јгент, чье состо€ние изменилось.
			agent_t & agent,
			//! “екущего состо€ни€ агента.
			const state_t & state )
		{
			m_state_listener.changed( agent, state );
		}

	private:
		agent_state_listener_t & m_state_listener;
};

//
// state_listener_controller
//

//! ‘унктор дл€ обработки смены состо€ний.
class state_change_functor
{
	public:
		state_change_functor(
			//! јгент, чье состо€ние изменилось.
			agent_t & agent,
			//! “екущего состо€ни€ агента.
			const state_t & state )
			:
				m_agent( agent ),
				m_state( state )
		{}

		void
		operator () ( agent_state_listener_ref_t & listener )
		{
			listener->changed( m_agent, m_state );
		}

	private:
		//! јгент, чье состо€ние изменилось.
		agent_t & m_agent;
		//! “екущего состо€ни€ агента.
		const state_t & m_state;
};

} /* ananymous namespace */

void
state_listener_controller_t::changed(
	//! јгент, чье состо€ние изменилось.
	agent_t & agent,
	//! “екущего состо€ни€ агента.
	const state_t & state )
{
	state_change_functor scf( agent, state );

	std::for_each(
		m_listeners.begin(),
		m_listeners.end(),
		scf );
}

void
state_listener_controller_t::so_add_nondestroyable_listener(
	agent_state_listener_t & state_listener )
{
	add_listener( agent_state_listener_ref_t(
		new nondestroyable_listener_item_t( state_listener ) ) );
}

void
state_listener_controller_t::so_add_destroyable_listener(
	agent_state_listener_unique_ptr_t state_listener )
{
	add_listener( agent_state_listener_ref_t(
		new destroyable_listener_item_t(
			std::move( state_listener ) ) ) );
}

inline void
state_listener_controller_t::add_listener(
	const agent_state_listener_ref_t & agent_state_listener_ref )
{
	m_listeners.push_back( agent_state_listener_ref );
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
