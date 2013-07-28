/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Ѕазовый класс слушател€ состо€ний агента.
*/

#if !defined( _SO_5__RT__AGENT_STATE_LISTENER_HPP_ )
#define _SO_5__RT__AGENT_STATE_LISTENER_HPP_

#include <memory>

#include <cpp_util_2/h/nocopy.hpp>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/state.hpp>

namespace so_5
{

namespace rt
{

class state_t;

//
// agent_state_listener_t
//

//!	Ѕазовый класс "слушателей" состо€ни€ агента.
/*!
	¬ некоторых случа€х нужно определ€ть моменты смены состо€ни€
	агента. Ќапример, при создании средств мониторинга приложени€.
	ƒл€ этих случаев предназначены "слушатели" состо€ни€ агента.

	јгенту может быть назначено любое количество "слушателей".
	јгент, после успешной смены своего состо€ни€, в методе so_change_state()
	вызовет у каждого из них метод changed().

	\note ¬ажно, при реализации слушател€ избегать рекурсии,
	когда слушатель напр€мую и опосредованно может приводить
	к смене заданным агентом его состо€ни€.
*/
class SO_5_TYPE agent_state_listener_t
	:
		private cpp_util_2::nocopy_t
{
	public:
		virtual ~agent_state_listener_t();

		//! ¬ызываетс€ после успешной смены состо€ни€ агента.
		virtual void
		changed(
			//! јгент, чье состо€ние изменилось.
			agent_t & agent,
			//! “екущее состо€ни€ агента.
			const state_t & state ) = 0;
};

//! ”мный указатель на слушател€
typedef std::unique_ptr< agent_state_listener_t >
	agent_state_listener_unique_ptr_t;

//! ”мна€ ссылка на agent_state_listener_t.
typedef std::shared_ptr< agent_state_listener_t >
	agent_state_listener_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
