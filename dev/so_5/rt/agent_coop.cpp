/*
	SObjectizer 5.
*/

#include <exception>
#include <so_5/h/exception.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5/rt/h/agent.hpp>

#include <so_5/rt/h/agent_coop.hpp>

namespace so_5
{

namespace rt
{

agent_coop_t::agent_coop_t(
	const nonempty_name_t & name,
	disp_binder_unique_ptr_t coop_disp_binder,
	impl::so_environment_impl_t & env_impl )
	:
		m_coop_name( name.query_name() ),
		m_lock( env_impl.create_agent_coop_mutex() ),
		m_agents_are_undefined( false ),
		m_coop_disp_binder( std::move(coop_disp_binder) ),
		m_so_environment_impl( env_impl ),
		m_working_agents_count( 0 )
{
}

agent_coop_unique_ptr_t
agent_coop_t::create_coop(
	const nonempty_name_t & name,
	disp_binder_unique_ptr_t coop_disp_binder,
	impl::so_environment_impl_t & env_impl )
{
	return agent_coop_unique_ptr_t( new agent_coop_t(
		name, std::move(coop_disp_binder), env_impl ) );
}

agent_coop_t::~agent_coop_t()
{
	m_so_environment_impl.destroy_agent_coop_mutex( m_lock );
}

const std::string &
agent_coop_t::query_coop_name() const
{
	return m_coop_name;
}


void
agent_coop_t::do_add_agent(
	const agent_ref_t & agent_ref )
{
	m_agent_array.push_back(
		agent_with_disp_binder_t( agent_ref, m_coop_disp_binder ) );
}

void
agent_coop_t::do_add_agent(
	const agent_ref_t & agent_ref,
	disp_binder_unique_ptr_t disp_binder )
{
	disp_binder_ref_t dbinder( disp_binder.release() );

	if( nullptr == dbinder.get() || nullptr == agent_ref.get() )
		throw exception_t(
			"zero ptr to agent or disp binder",
			rc_coop_has_references_to_null_agents_or_binders );

	m_agent_array.push_back(
		agent_with_disp_binder_t( agent_ref, dbinder ) );
}

void
agent_coop_t::bind_agents_to_coop()
{
	agent_array_t::iterator it = m_agent_array.begin();
	agent_array_t::iterator it_end = m_agent_array.end();

	for(; it != it_end; ++it )
	{
		it->m_agent_ref->bind_to_coop( *this );
	}
}

void
agent_coop_t::define_all_agents()
{
	agent_array_t::iterator it = m_agent_array.begin();
	agent_array_t::iterator it_end = m_agent_array.end();

	try
	{
		for(; it != it_end; ++it )
		{
			it->m_agent_ref->define_agent();
		}
	}
	catch( const std::exception & ex )
	{
		undefine_some_agents( it );
		throw;
	}
}

void
agent_coop_t::undefine_all_agents()
{
	undefine_some_agents( m_agent_array.end() );
}

void
agent_coop_t::undefine_some_agents(
	agent_array_t::iterator it )
{
	{
		// A flag of agent undefinition should be set.
		ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
		m_agents_are_undefined = true;
	}

	for( auto it_begin = m_agent_array.begin(); it != it_begin; )
	{
		--it;
		it->m_agent_ref->undefine_agent();
	}
}

void
agent_coop_t::bind_agents_to_disp()
{
	agent_array_t::iterator it;
	agent_array_t::iterator it_begin = m_agent_array.begin();
	agent_array_t::iterator it_end = m_agent_array.end();

	try
	{
		for( it = it_begin; it != it_end; ++it )
		{
			it->m_binder->bind_agent(
				m_so_environment_impl, it->m_agent_ref );
		}
	}
	catch( const std::exception & ex )
	{
		unbind_agents_from_disp( it );

		// Because all agents are defined at this point then
		// they should be undefined.
		undefine_all_agents();

		throw;
	}

	// A total count of all active agents should be set because
	// all agents are successfully registered.
	m_working_agents_count = m_agent_array.size();
}

inline void
agent_coop_t::unbind_agents_from_disp(
	agent_array_t::iterator it )
{
	for( auto it_begin = m_agent_array.begin(); it != it_begin; )
	{
		--it;
		it->m_binder->unbind_agent(
			m_so_environment_impl, it->m_agent_ref );
	}
}

void
agent_coop_t::agent_finished()
{
	// If it is the last working agent then Environment should be
	// informed that the cooperation is ready to be deregistered.
	if( 0 == --m_working_agents_count )
	{
		m_so_environment_impl.ready_to_deregister_notify( this );
	}
}

void
agent_coop_t::final_deregister_coop()
{
	unbind_agents_from_disp( m_agent_array.end() );

	m_so_environment_impl.final_deregister_coop( m_coop_name );
}

} /* namespace rt */

} /* namespace so_5 */

