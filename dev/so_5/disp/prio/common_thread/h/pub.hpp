/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.8
 * \file
 * \brief Functions for creating and binding of the single thread dispatcher
 * with priority support.
 */

#pragma once

#include <string>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/disp_binder.hpp>

#include <so_5/disp/prio/h/priority.hpp>

namespace so_5 {

namespace disp {

namespace prio {

namespace common_thread {

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

/*!
 * \since v.5.5.8
 * \brief Create a private %common_thread dispatcher.
 *
 * \par Usage sample
\code
auto common_thread_disp = so_5::disp::prio::common_thread::create_private_disp( env );

auto coop = env.create_coop( so_5::autoname,
	// The main dispatcher for that coop will be
	// private common_thread dispatcher.
	// Main binder will use the lowest priority for agents.
	common_thread_disp->binder( so_5::disp::prio::p0 ) );
\endcode
 */
inline private_dispatcher_handle_t
create_private_disp( so_5::rt::environment_t & env )
	{
		return create_private_disp( env, std::string() );
	}

//! Create a dispatcher binder object.
SO_5_FUNC so_5::rt::disp_binder_unique_ptr_t
create_disp_binder(
	//! Name of the dispatcher to be bound to.
	const std::string & disp_name,
	//! Priority for agents to be bound.
	priority_t priority );

} /* namespace common_thread */

} /* namespace prio */

} /* namespace disp */

} /* namespace so_5 */

