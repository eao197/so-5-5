/*
	SObjectizer 5.
*/

/*!
	\file
	\brief ќчередь за€вок на дерегистрацию коопераций.
*/

#if !defined( _SO_5__RT__IMPL__COOP_DEREG__DEREG_DEMAND_QUEUE_HPP_ )
#define _SO_5__RT__IMPL__COOP_DEREG__DEREG_DEMAND_QUEUE_HPP_

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

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
// dereg_demand_queue_t
//

//! ќчередь за€вок.
/*!
	“ак же хранит признак необходимости завершени€
	работы.

	ѕредназначена дл€ использовани€ несколькими
	нит€ми одновременно.
*/
class dereg_demand_queue_t
{
	public:
		typedef std::vector< agent_coop_t* > dereg_demand_container_t;

		dereg_demand_queue_t();
		~dereg_demand_queue_t();

		//! ѕоместить за€вку на исполнение событий в очередь.
		void
		push( agent_coop_t * coop );

		//! ¬з€ть первую за€вку.
		/*!
			≈сли за€вок в очереди нет, то текуща€ нить
			засыпает до по€влени€ за€вок в очереди, либо
			до выставлени€ признака завершени€ работы.

			 огда работа завершена, то пиремник не будет содержать за€вок.
		*/
		void
		pop(
			/*! ѕриемник зa€вок. */
			dereg_demand_container_t & demands );

		//! Ќачать обслуживание за€вок.
		void
		start_service();

		//! ќстановить обслуживание за€вок.
		void
		stop_service();

	private:
		//!  онтейнер очереди.
		dereg_demand_container_t m_demands;

		//! —инхронизаци€.
		//! \{
		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_not_empty;
		//! \}

		//! ‘лаг - обслуживать ли клиентов очереди?.
		/*! ѕринимает значени€:
			true - надо продолжать работу - обслуживать методы push/pop.
			false - прекратить обслуживание.
		*/
		bool m_in_service;
};

} /* namespace coop_dereg */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
