/*
	SObjectizer 5.
*/

/*!
	\file
	\brief  онтроллер слушателей состо€ни€ агента.

	 онтроллер слушателей состо€ни€ агента хранит весь список
	слушателей и при необходимости инициирует обработку
	смены состо€ни€ агентами всеми слушател€ми.

	“.к. ¬ладение слушател€ми может принадлежать агенту а может и нет,
	то список состоит из элементов двух типов:
	те которые уничтажают слушателс€ при собственном уничтожении,
	и те которые при собственном уничтожении ничего не делают.
	ќба типа элементов представл€ют собой реализацию agent_state_listener_t,
	которые передают обработку смены состо€ни€ подконтрольным им слушател€м.
*/

#if !defined( _SO_5__RT__IMPL__STATE_LISTENER_CONTROLLER_HPP_ )
#define _SO_5__RT__IMPL__STATE_LISTENER_CONTROLLER_HPP_

#include <vector>

#include <so_5/rt/h/agent_state_listener.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// state_listener_controller
//

//!  онтроллер слушателей состо€ни€ агента.
class state_listener_controller_t
{
	public:
		state_listener_controller_t()
		{}

		~state_listener_controller_t()
		{}

		//! ќбработка смены состо€ни€.
		void
		changed(
			//! јгент, чье состо€ние изменилось.
			agent_t & agent,
			//! “екущего состо€ни€ агента.
			const state_t & state );

		//! ƒобавить слушател€,
		//! врем€ жизни которого не контролируетс€.
		void
		so_add_nondestroyable_listener(
			agent_state_listener_t & state_listener );

		//! ƒобавить слушателс€,
		//! владение которым забираетс€.
		void
		so_add_destroyable_listener(
			agent_state_listener_unique_ptr_t state_listener );

	private:
		//! ƒобавить слушател€.
		void
		add_listener(
			const agent_state_listener_ref_t & agent_state_listener_ref );

		//! “ип списка слушателей.
		typedef std::vector<
				agent_state_listener_ref_t >
			agent_state_listener_array_t;

		//! —лушатели состо€ний агента.
		agent_state_listener_array_t m_listeners;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
