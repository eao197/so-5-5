/*
	SObjectizer 5.
*/

#include <so_5/disp/one_thread/impl/h/disp_binder.hpp>
#include <so_5/disp/one_thread/impl/h/disp.hpp>

#include <so_5/h/exception.hpp>
#include <so_5/rt/h/agent.hpp>

namespace so_5
{

namespace disp
{

namespace one_thread
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
	so_5::rt::impl::so_environment_impl_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	// If the dispatcher is found then the object should be bound to it.
	if( disp_ref.get() )
	{
		// It should be exactly our dispatcher.
		dispatcher_t * disp = dynamic_cast< dispatcher_t * >( disp_ref.get() );

		if( nullptr == disp )
			throw so_5::exception_t(
				"disp type mismatch for disp \"" + m_disp_name +
						"\", expected one_thread disp",
				rc_disp_type_mismatch );

		so_5::rt::agent_t::bind_to_dispatcher(
				*agent_ref, *disp, *disp );
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
	so_5::rt::impl::so_environment_impl_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
}

} /* namespace impl */

} /* namespace one_thread */

} /* namespace disp */

} /* namespace so_5 */
