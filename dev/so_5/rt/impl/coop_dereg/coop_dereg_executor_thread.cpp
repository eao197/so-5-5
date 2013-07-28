/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Thread_Manager.h>

#include <so_5/h/log_err.hpp>

#include <so_5/rt/impl/coop_dereg/h/coop_dereg_executor_thread.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

namespace coop_dereg
{

//
// coop_dereg_executor_thread_t
//

coop_dereg_executor_thread_t::coop_dereg_executor_thread_t()
{
}

coop_dereg_executor_thread_t::~coop_dereg_executor_thread_t()
{
}

void
coop_dereg_executor_thread_t::start()
{
	// Выставляем очереди флаг, что надо работать.
	m_dereg_demand_queue.start_service();

	SO_5_ABORT_ON_ACE_ERROR(
		ACE_Thread_Manager::instance()->spawn(
			entry_point,
			this,
			THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
			&m_tid ) );
}

void
coop_dereg_executor_thread_t::shutdown()
{
	m_dereg_demand_queue.stop_service();
}


void
coop_dereg_executor_thread_t::wait()
{
	ACE_Thread_Manager::instance()->join( m_tid );
}

void
coop_dereg_executor_thread_t::push_dereg_demand(
	agent_coop_t * coop )
{
	m_dereg_demand_queue.push( coop );
}

void
exec_final_coop_dereg( agent_coop_t * coop )
{
	agent_coop_t::call_final_deregister_coop( coop );
}

void
coop_dereg_executor_thread_t::body()
{
	dereg_demand_queue_t::dereg_demand_container_t demands;
	do
	{
		demands.clear();
		m_dereg_demand_queue.pop( demands );

		std::for_each(
			demands.begin(),
			demands.end(),
			exec_final_coop_dereg );
	}
	while( !demands.empty() );
}

ACE_THR_FUNC_RETURN
coop_dereg_executor_thread_t::entry_point( void * self_object )
{
	coop_dereg_executor_thread_t * coop_dereg_executor_thread =
		reinterpret_cast<coop_dereg_executor_thread_t *>( self_object );

	coop_dereg_executor_thread->body();

	return 0;
}

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
