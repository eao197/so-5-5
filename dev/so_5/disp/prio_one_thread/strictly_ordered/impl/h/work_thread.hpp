/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.8
 * \file
 * \brief A working thread for dispatcher with one common working
 * thread and support of demands priority.
 */

#pragma once

#include <so_5/disp/prio_one_thread/strictly_ordered/impl/h/demand_queue.hpp>

#include <so_5/h/current_thread_id.hpp>

#include <thread>

namespace so_5 {

namespace disp {

namespace prio_one_thread {

namespace strictly_ordered {

namespace impl {

//
// work_thread_t
//
/*!
 * \since v.5.5.8
 * \brief A working thread for dispatcher with one common working
 * thread and support of demands priority.
 */
class work_thread_t
	{
	public :
		//! Initializing constructor.
		work_thread_t( demand_queue_t & queue )
			:	m_queue( queue )
			{}

		void
		start()
			{
				m_thread = std::thread( [this]() { body(); } );
			}

		void
		join()
			{
				m_thread.join();
			}

	private :
		//! Demands queue to work for.
		demand_queue_t & m_queue;

		//! Thread ID to be passed to event handlers.
		/*!
		 * \note Receives actual value at the start of body() method.
		 */
		so_5::current_thread_id_t m_thread_id;

		//! Thread object.
		std::thread m_thread;

		void
		body()
			{
				m_thread_id = so_5::query_current_thread_id();

				try
					{
						for(;;)
							process_demand( m_queue.pop() );
					}
				catch( const demand_queue_t::shutdown_ex_t & )
					{}
			}

		void
		process_demand( demand_unique_ptr_t demand )
			{
//FIXME: it could rewritten as:
// demand->call_handler( m_thread_id );
				(*(demand->m_demand_handler))( m_thread_id, *demand );
			}
	};

} /* namespace impl */

} /* namespace strictly_ordered */

} /* namespace prio_one_thread */

} /* namespace disp */

} /* namespace so_5 */

