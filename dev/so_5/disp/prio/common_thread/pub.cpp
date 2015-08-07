/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.8
 * \file
 * \brief Functions for creating and binding of the single thread dispatcher
 * with priority support.
 */

#include <so_5/disp/prio/common_thread/h/pub.hpp>

#include <so_5/disp/prio/common_thread/impl/h/demand_queue.hpp>
#include <so_5/disp/prio/common_thread/impl/h/work_thread.hpp>

namespace so_5 {

namespace disp {

namespace prio {

namespace common_thread {

//
// dispatcher_t
//
/*!
 * \since v.5.5.8
 * \brief An actual implementation of dispatcher with one working
 * thread and support of demand priorities.
 */
class dispatcher_t : public so_5::rt::dispatcher_t
	{
	public:
		dispatcher_t()
			:	m_work_thread{ m_demand_queue }
			{}

		//! \name Implementation of so_5::rt::dispatcher methods.
		//! \{
		virtual void
		start( so_5::rt::environment_t & env ) override
			{
//FIXME: data source must be started here!
//				m_data_source.start( env );

				so_5::details::do_with_rollback_on_exception(
						[this] { m_work_thread.start(); },
						[this] {
//FIXME: data source must be stopped here!
// m_data_source.stop();
						} );
			}

		virtual void
		shutdown() override
			{
				m_demand_queue.stop();
			}

		virtual void
		wait() override
			{
				m_work_thread.join();

//FIXME: data source must be stopped here!
//				m_data_source.stop();
			}

		virtual void
		set_data_sources_name_base(
			const std::string & name_base ) override
			{
//FIXME: base part of data source name must be set here!
//				m_data_source.set_data_sources_name_base( name_base, this );
			}
		//! \}

		/*!
		 * \since v.5.4.0
		 * \brief Get a binding information for an agent.
		 */
		so_5::rt::event_queue_t *
		get_agent_binding( priority_t priority )
			{
				return &m_demand_queue.event_queue_by_priority( priority );
			}

	private:
		//! Demand queue for the dispatcher.
		impl::demand_queue_t m_demand_queue;

		//! Working thread for the dispatcher.
		impl::work_thread_t m_work_thread;
	};

//FIXME: implement this!
#if 0
//
// private_dispatcher_t
//

/*!
 * \since v.5.5.8
 * \brief An interface for %common_thread private dispatcher.
 */
class SO_5_TYPE private_dispatcher_t : public so_5::atomic_refcounted_t
	{
	public :
		virtual ~private_dispatcher_t();

		//! Create a binder for that private dispatcher.
		virtual so_5::rt::disp_binder_unique_ptr_t
		binder(
			//! Priority of agent to be bound.
			priority_t agent_priority ) = 0;
	};

/*!
 * \since v.5.5.8
 * \brief A handle for the %one_thread private dispatcher.
 */
using private_dispatcher_handle_t =
	so_5::intrusive_ptr_t< private_dispatcher_t >;

//! Create a dispatcher.
SO_5_FUNC so_5::rt::dispatcher_unique_ptr_t
create_disp();

/*!
 * \since v.5.5.8
 * \brief Create a private %common_thread dispatcher.
 *
 * \par Usage sample
\code
auto common_thread_disp = so_5::disp::prio::common_thread::create_private_disp(
	env,
	"request_processor" );

auto coop = env.create_coop( so_5::autoname,
	// The main dispatcher for that coop will be
	// private common_thread dispatcher.
	// Main binder will use the lowest priority for agents.
	common_thread_disp->binder( so_5::disp::prio::p0 ) );
\endcode
 */
SO_5_FUNC private_dispatcher_handle_t
create_private_disp(
	//! SObjectizer Environment to work in.
	so_5::rt::environment_t & env,
	//! Value for creating names of data sources for
	//! run-time monitoring.
	const std::string & data_sources_name_base );

//! Create a dispatcher binder object.
SO_5_FUNC so_5::rt::disp_binder_unique_ptr_t
create_disp_binder(
	//! Name of the dispatcher to be bound to.
	const std::string & disp_name,
	//! Priority for agents to be bound.
	priority_t priority );

#endif

} /* namespace common_thread */

} /* namespace prio */

} /* namespace disp */

} /* namespace so_5 */

