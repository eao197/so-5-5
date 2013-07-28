/*
	SObjectizer 5.
*/

/*!
	\file
	\brief ќчередь за€вок на дерегистрацию коопераций.
*/

#if !defined( _SO_5__RT__IMPL__COOP_DEREG__COOP_DEREG_EXECUTOR_THREAD_HPP_ )
#define _SO_5__RT__IMPL__COOP_DEREG__COOP_DEREG_EXECUTOR_THREAD_HPP_

#include <so_5/rt/impl/coop_dereg/h/dereg_demand_queue.hpp>

#include <so_5/rt/h/agent_coop.hpp>

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

//!  ласс нити дерегистратора коопераций.
/*!
	ƒл€ того чтобы отв€зка агентов от диспетчеров происходила не
	на нити одного из агентов кооперации, а на независимой от
	коопераций нити, при поступлении момента, когда все агенты
	кооперации завершили выполнение своих действий, коопераци€
	ставит себ€ в очередь на дерегистрацию, котора€ обрабатываетс€
	на служебной нити agent_core.
*/
class coop_dereg_executor_thread_t
{
	public:
		coop_dereg_executor_thread_t();
		~coop_dereg_executor_thread_t();

		//! «апустить нить.
		void
		start();

		//! ƒать сигнал к останову работы.
		void
		shutdown();

		//! ќжидать завершени€ работы.
		void
		wait();

		//! ѕоставить кооперацию в очередь на дерегистрацию.
		void
		push_dereg_demand(
			agent_coop_t * coop );

	protected:
		//! ќсновное тело циклической работы.
		void
		body();

		//! “очка входа в нить дл€ ACE_Thread_Manager.
		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

	private:
		//! ќчередь за€вок на дерегистрацию коопераций.
		dereg_demand_queue_t m_dereg_demand_queue;

		//! »дентификатор нити, созданной дл€ данного объекта.
		/*!
			\note «начение актуально только после вызова start().
		*/
		ACE_thread_t m_tid;
};

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
