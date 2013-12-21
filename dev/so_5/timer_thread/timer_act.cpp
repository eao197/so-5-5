/*
	SObjectizer 5.
*/

#include <so_5/timer_thread/h/timer_act.hpp>

#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace timer_thread
{

timer_act_t::timer_act_t(
	const std::type_index & type_index,
	const rt::mbox_ref_t & mbox,
	const rt::message_ref_t & msg,
	unsigned int delay,
	unsigned int period )
	:
		m_type_index( type_index ),
		m_mbox( mbox ),
		m_msg( msg ),
		m_delay( delay ),
		m_period( period )
{
}

timer_act_t::~timer_act_t()
{
}

void
timer_act_t::exec()
{
	m_mbox->deliver_message( m_type_index, m_msg );
}

bool
timer_act_t::is_periodic() const
{
	return 0 != m_period;
}

unsigned int
timer_act_t::query_delay() const
{
	return m_delay;
}

unsigned int
timer_act_t::query_period() const
{
	return m_period;
}

} /* namespace timer_thread */

} /* namespace so_5 */

