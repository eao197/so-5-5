/*
	SObjectizer 5.
*/

#include <so_5/disp/active_obj/impl/h/disp_binder.hpp>
#include <so_5/disp/active_obj/impl/h/disp.hpp>

#include <so_5/h/exception.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace disp
{

namespace active_obj
{

namespace impl
{

disp_binder_t::disp_binder_t(
	const std::string & disp_name )
	:
		m_disp_name( disp_name )
{
}

disp_binder_t::~disp_binder_t()
{
}

void
disp_binder_t::bind_agent(
	so_5::rt::so_environment_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	// If the dispatcher is found then the agent should be bound to it.
	if( disp_ref.get() )
	{
		// It should be an active objects dispatcher.
		dispatcher_t * disp = dynamic_cast< dispatcher_t * >( disp_ref.get() );

		if( nullptr == disp )
			throw so_5::exception_t(
				"disp type mismatch for disp \"" + m_disp_name +
					"\", expected active_obj disp",
				rc_disp_type_mismatch );

		auto ctx = disp->create_thread_for_agent( *agent_ref );

		try
		{
			agent_ref->so_bind_to_dispatcher( ctx.first, *ctx.second );
		}
		catch( ... )
		{
			// Dispatcher for the agent should be removed.
			disp->destroy_thread_for_agent( *agent_ref );
			throw;
		}
	}
	else
	{
		throw so_5::exception_t(
			"dispatcher with name \"" + m_disp_name + "\" not found",
			rc_named_disp_not_found );
	}
}

void
disp_binder_t::unbind_agent(
	so_5::rt::so_environment_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	if( disp_ref.get() )
	{
		// This should be an active_obj dispatcher because binding
		// was successfully passed earlier.
		dispatcher_t & disp = dynamic_cast< dispatcher_t & >( *disp_ref );

		disp.destroy_thread_for_agent( *agent_ref );
	}
}

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */
