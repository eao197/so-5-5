/*
	SObjectizer 5.
*/

#include <so_5/rt/impl/coop_dereg/h/dereg_demand_queue.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

namespace coop_dereg
{


dereg_demand_queue_t::dereg_demand_queue_t()
	:
		m_not_empty( m_lock )
{
}

dereg_demand_queue_t::~dereg_demand_queue_t()
{
}

void
dereg_demand_queue_t::push( agent_coop_t * coop )
{
	bool was_empty;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

		if( m_in_service )
		{
			was_empty = m_demands.empty();
			m_demands.push_back( coop );
		}
	}

	if( was_empty )
		m_not_empty.signal();
}

void
dereg_demand_queue_t::pop(
	dereg_demand_container_t & demands )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	while( true )
	{
		if( m_in_service && m_demands.empty() )
		{
			// We should wait for some event.
			m_not_empty.wait();
		}
		else
		{
			if( !m_demands.empty() )
				demands.swap( m_demands );
			break;
		}
	}
}

void
dereg_demand_queue_t::start_service()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	m_in_service = true;
}

void
dereg_demand_queue_t::stop_service()
{
	bool need_wakeup_signal = false;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

		m_in_service = false;
		// If demands queue is empty then someone could wait on
		// object lock. This thread should be waked up.
		need_wakeup_signal = m_demands.empty();
	}

	if( need_wakeup_signal )
		m_not_empty.signal();
}

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
