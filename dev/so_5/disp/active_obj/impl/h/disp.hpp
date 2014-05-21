/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Real class of the active objects dispatcher.
*/

#if !defined( _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_HPP_ )
#define _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_HPP_

#include <map>

#include <ace/Thread_Mutex.h>

#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/exec_ctx.hpp>

namespace so_5 {

namespace rt
{
	class agent_t;

} /* namespace rt */


namespace disp
{

namespace active_obj
{

namespace impl
{

//
// dispatcher_t
//

/*!
	\brief Active objects dispatcher.
*/
class dispatcher_t
	:	public so_5::rt::dispatcher_t
	,	public so_5::rt::exec_ctx_t
{
	public:
		dispatcher_t();
		virtual ~dispatcher_t();

		//! \name Implemetation of so_5::rt::dispatcher methods.
		//! \{

		virtual void
		start();

		virtual void
		shutdown();

		virtual void
		wait();

		/*!
		 * \attention Shall not be called directly for this dispatcher.
		 */
		virtual void
		put_event_execution_request(
			so_5::rt::agent_t * agent_ptr,
			unsigned int event_count );
		//! \}

		//! Creates a new thread for the agent specified.
		so_5::rt::dispatcher_t &
		create_disp_for_agent( const so_5::rt::agent_t & agent );

		//! Destroys the thread for the agent specified.
		void
		destroy_disp_for_agent( const so_5::rt::agent_t & agent );

	private:
		//! Typedef for mapping from agents for their single thread dispatchers.
		typedef std::map<
				const so_5::rt::agent_t *,
				so_5::rt::dispatcher_ref_t >
			agent_disp_map_t;

		//! A map from agents to single thread dispatchers.
		agent_disp_map_t m_agent_disp;

		//! Shutdown flag.
		bool m_shutdown_started;

		//! This object lock.
		ACE_Thread_Mutex m_lock;
};

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */

#endif

