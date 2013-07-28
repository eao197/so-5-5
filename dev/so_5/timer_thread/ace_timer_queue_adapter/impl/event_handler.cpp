/*
	SObjectizer 5.
*/

#include <so_5/timer_thread/ace_timer_queue_adapter/impl/h/event_handler.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

namespace impl
{

//
// timer_event_handler_t
//

timer_event_handler_t::timer_event_handler_t(
		timer_act_utilizator_t & utilizator )
	:
		m_utilizator( utilizator )
{
}

timer_event_handler_t::~timer_event_handler_t()
{
}

int
timer_event_handler_t::handle_timeout(
	const ACE_Time_Value &,
	const void * raw_act )
{

	timer_act_t * act = reinterpret_cast< timer_act_t * >(
			const_cast< void * >( raw_act ) );

	act->exec();

	m_utilizator.utilize_timer_act( act );

	return 0;
}

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */
