/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Implementation of the timer_thread on top of
		the ACE_Thread_Timer_Queue_Adapter.
*/

#if !defined( _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__TIMER_THREAD_HPP_ )
#define _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__IMPL__TIMER_THREAD_HPP_

#include <memory>
#include <map>

#include <ace/Timer_Queue_Adapters.h>
#include <ace/Timer_List.h>

#include <so_5/timer_thread/h/timer_thread.hpp>

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
// timer_thread_t
//

/*!
	\brief Implementation of the timer_thread on top of the ACE_Thread_Timer_Queue_Adapter.
 */
class timer_thread_t
	:
		public so_5::timer_thread::timer_thread_t,
		public timer_act_utilizator_t
{
		//! Typedef for the base type.
		typedef so_5::timer_thread::timer_thread_t base_type_t;
	public:

		//! Auxiliary class for storing timer ID in ACE and SObjectizer.
		struct timer_keys_t
		{
			timer_keys_t( timer_id_t timer_id, long ace_id )
				:
					m_timer_id( timer_id ),
					m_ace_id( ace_id )
			{}

			timer_id_t	m_timer_id;
			long		m_ace_id;
		};

		//! Constructor.
		timer_thread_t();

		virtual ~timer_thread_t();

		/*!
		 * \name Implementation of the so_5::timer_thread::timer_thread.
		 * \{
		 */

		/*!
		 * Calls ACE_Thread_Timer_Queue_Adapter::activate().
		 */
		virtual void
		start();

		/*!
		 * Calls ACE_Thread_Timer_Queue_Adapter::deactivate().
		 */
		virtual void
		shutdown();

		/*!
		 * Calss ACE_Thread_Timer_Queue_Adapter::wait().
		 */
		virtual void
		wait();

		/*!
		 * Assigns own ID to the event and creates Timer ACT.
		 * Registers an event in the ACE_Thread_Timer_Queue_Adapter and stores
		 * its description in the m_agent_demands.
		 *
		 * \note Method blocks the timer thread.
		 */
		virtual timer_id_t
		schedule_act(
			timer_act_unique_ptr_t timer_act );

		virtual void
		cancel_act(
			timer_id_t msg_id );

		/*!
		 * \}
		 */

		/*!
		 * \brief Quick timer event cancelation.
		 *
		 * Mutex of the timer event thread isn't acquired.
		 * 
		 * Method will not remove timer event from the map.
		 *
		 * \note Use only in the timer_thread shutdown.
		 */
		virtual void
		quick_cancel_act(
			const timer_keys_t & timer_keys );

		/*!
		 * \name Implementation of the timer_act_utilizator_t.
		 * \{
		 */

		virtual void
		utilize_timer_act(
			timer_act_t * timer_act );

		/*!
		 * \}
		 */

	private:
		//! Cancel all timer events.
		/*!
		 * \attention Must be used only at the end of the thread work.
		 */
		void
		cancel_all();

		//! Object lock.
		ACE_SYNCH_RECURSIVE_MUTEX &
		mutex();

		//! Typedef for the ACE-based timer implementation.
		typedef ACE_Thread_Timer_Queue_Adapter< ACE_Timer_List >
			adapter_t;

		//! ACE-based timer implementation.
		/*!
		 * This object should be destroyed before the m_event_handler.
		 * It is because of calls to the m_event_handler are possible
		 * during this object destruction.
		 */
		std::unique_ptr< adapter_t > m_timer_queue;

		/*!
		 * This event handler will be used for all timer events.
		 */
		std::unique_ptr< timer_event_handler_t > m_event_handler;

		//! Typedef for the map from the timer_act to IDs inside of ACE-based timer.
		typedef std::map<
				timer_act_t*,
				timer_keys_t >
			scheduled_act_to_id_map_t;

		//! Map of sheduled events.
		scheduled_act_to_id_map_t m_scheduled_act_to_id;

		//! Counter for the own ID generation.
		/*!
		 * A very simple approach is used. The 64-bit counter is used.
		 * It is assumed that the 64-bit counter is never overflow even
		 * in a very-very long program run.
		 */
		timer_id_t m_self_id_counter;

		//! Typedef for the map from the timer_id to the ace_id.
		typedef std::map<
				timer_id_t,
				long >
			timer_id_to_ace_id_map_t;

		//! Map of the own timer ID to IDs inside of ACE-based timer.
		timer_id_to_ace_id_map_t m_timer_id_to_ace_id;
	};

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

