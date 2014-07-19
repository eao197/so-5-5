/*
	SObjectizer 5.
*/

#include <so_5/disp/active_group/impl/h/disp_binder.hpp>
#include <so_5/disp/active_group/impl/h/disp.hpp>

#include <so_5/h/exception.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace disp
{

namespace active_group
{

namespace impl
{

disp_binder_t::disp_binder_t(
	const std::string & disp_name,
	const std::string & group_name )
	:
		m_disp_name( disp_name ),
		m_group_name( group_name )
{
}

disp_binder_t::~disp_binder_t()
{
}

so_5::rt::disp_binding_activator_t
disp_binder_t::bind_agent(
	so_5::rt::so_environment_t & env,
	so_5::rt::agent_ref_t agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	// If the dispatcher is found then agent should be bound to it.
	if( disp_ref.get() )
	{
		// It should be an active group dispatcher.
		dispatcher_t * disp = dynamic_cast< dispatcher_t * >( disp_ref.get() );

		if( nullptr == disp )
			throw so_5::exception_t(
				"disp type mismatch for disp \"" + m_disp_name +
				"\", expected active_group disp",
				rc_disp_type_mismatch );

		auto ctx = disp->query_thread_for_group( m_group_name );

		try
		{
			so_5::rt::disp_binding_activator_t activator =
				[agent_ref, ctx]() {
					agent_ref->so_bind_to_dispatcher( *ctx );
				};

			return activator;
		}
		catch( ... )
		{
			// Dispatcher for the agent should be removed.
			disp->release_thread_for_group( m_group_name );
			throw;
		}
	}

	throw so_5::exception_t(
		"dispatcher with name \"" + m_disp_name + "\" not found",
		rc_named_disp_not_found );
}

void
disp_binder_t::unbind_agent(
	so_5::rt::so_environment_t & env,
	so_5::rt::agent_ref_t agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	if( disp_ref.get() )
	{
		// This should be an active_group dispatcher because binding
		// was successfully passed earlier.
		dispatcher_t & disp = dynamic_cast< dispatcher_t & >( *disp_ref );

		disp.release_thread_for_group( m_group_name );
	}
}

} /* namespace impl */

} /* namespace active_group */

} /* namespace disp */

} /* namespace so_5 */

