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
	so_5::rt::impl::so_environment_impl_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	// Если есть такой диспетчер, то будем привязывать агента к нему.
	if( disp_ref.get() )
	{
		// Это должен быть именно active_obj::impl::dispatcher_t.
		dispatcher_t * disp = dynamic_cast< dispatcher_t * >( disp_ref.get() );

		if( nullptr == disp )
			throw so_5::exception_t(
				"disp type mismatch for disp \"" + m_disp_name + "\", expected active_obj disp",
				rc_disp_type_mismatch );

		so_5::rt::dispatcher_t & disp_for_agent =
			disp->create_disp_for_agent( *agent_ref );

		try
		{
			so_5::rt::agent_t::call_bind_to_disp(
				*agent_ref,
				disp_for_agent );
		}
		catch( ... )
		{
			// Если при привязке возникают ошибки,
			// то надо позаботиться, чтобы
			// созданный для агента диспетчер удалился.
			disp->destroy_disp_for_agent( *agent_ref );
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
	so_5::rt::impl::so_environment_impl_t & env,
	so_5::rt::agent_ref_t & agent_ref )
{
	so_5::rt::dispatcher_ref_t disp_ref =
		env.query_named_dispatcher( m_disp_name );

	if( disp_ref.get() )
	{
		// Это должен быть именно active_obj::impl::dispatcher_t,
		// иначе бы привязка не удалась бы и unbind_agent не вызывался.
		dispatcher_t & disp = dynamic_cast< dispatcher_t & >( *disp_ref );

		// Удаляем диспетчер.
		disp.destroy_disp_for_agent( *agent_ref );
	}
}

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */
