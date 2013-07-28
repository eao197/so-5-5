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

		// Если надо обслуживать.
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
			// Нужно ждать наступления
			// какого-нибудь события.
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

	// Выставляем флаг - начать работу очереди.
	m_in_service = true;
}

void
dereg_demand_queue_t::stop_service()
{
	bool need_signal_not_empty;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

		// Выставляем флаг - прекратить работу очереди.
		m_in_service = false;
		need_signal_not_empty = m_demands.empty();
	}

	// Если кто-то ждет - сигналим.
	if( need_signal_not_empty)
		m_not_empty.signal();
}

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
