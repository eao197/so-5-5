/*
	SObjectizer 5.
*/

#include <so_5/h/exception.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace rt
{

//
// disp_binder_t
//

disp_binder_t::disp_binder_t()
{
}

disp_binder_t::~disp_binder_t()
{
}

namespace /* ananymous */
{

//
// default_disp_binder_t
//

//! Default dispatcher binder implementation.
class default_disp_binder_t
	:
		public disp_binder_t
{
	public:
		default_disp_binder_t();
		virtual ~default_disp_binder_t();

		virtual void
		bind_agent(
			impl::so_environment_impl_t & env,
			agent_ref_t & agent_ref );

		virtual void
		unbind_agent(
			impl::so_environment_impl_t & env,
			agent_ref_t & agent_ref );
};

default_disp_binder_t::default_disp_binder_t()
{
}

default_disp_binder_t::~default_disp_binder_t()
{
}

void
default_disp_binder_t::bind_agent(
	impl::so_environment_impl_t & env,
	agent_ref_t & agent_ref )
{
	agent_t::call_bind_to_disp(
		*agent_ref,
		env.query_default_dispatcher() );
}

void
default_disp_binder_t::unbind_agent(
	impl::so_environment_impl_t & env,
	agent_ref_t & agent_ref )
{
}

} /* ananymous namespace */

SO_5_EXPORT_FUNC_SPEC( disp_binder_unique_ptr_t )
create_default_disp_binder()
{
	return disp_binder_unique_ptr_t( new default_disp_binder_t );
}

} /* namespace rt */

} /* namespace so_5 */

