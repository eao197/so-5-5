/*
	SObjectizer 5.
*/

#include <cstdlib>

#include <ace/Guard_T.h>

#include <so_5/rt/h/agent.hpp>

#include <so_5/rt/impl/h/local_event_queue.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{


//
// local_event_queue_t
//

local_event_queue_t::local_event_queue_t(
	util::mutex_pool_t< ACE_Thread_Mutex > & mutex_pool )
	:
		m_mutex_pool( mutex_pool ),
		m_lock( m_mutex_pool.allocate_mutex() )
{
}

local_event_queue_t::~local_event_queue_t()
{
	// Mutex should be returned to pool.
	m_mutex_pool.deallocate_mutex( m_lock );
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

