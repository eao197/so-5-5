/*
	SObjectizer 5.
*/

#include <exception>

#include <so_5/h/exception.hpp>
#include <so_5/h/log_err.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5/rt/h/agent.hpp>

#include <so_5/rt/h/agent_coop.hpp>

namespace so_5
{

namespace rt
{

//
// coop_notificators_container_t
//
coop_notificators_container_t::coop_notificators_container_t()
{}

coop_notificators_container_t::~coop_notificators_container_t()
{}

void
coop_notificators_container_t::add(
	const coop_notificator_t & notificator )
{
	m_notificators.push_back( notificator );
}

void
coop_notificators_container_t::call_all(
	so_environment_t & env,
	const std::string & coop_name ) const
{
	for( auto i = m_notificators.begin(); i != m_notificators.end(); ++i )
	{
		// Exceptions should not go out.
		try
		{
			(*i)( env, coop_name );
		}
		catch( const std::exception & x )
		{
			ACE_ERROR(
					(LM_ERROR,
					 SO_5_LOG_FMT( "on notification for coop '%s' exception: %s" ),
					 coop_name.c_str(),
					 x.what() ) );
		}
	}
}

//
// agent_coop_t
//

agent_coop_t::~agent_coop_t()
{
	// Initiate deleting of agents by hand to guarantee that
	// agents will be destroyed before return from agent_coop_t
	// destructor.
	//
	// NOTE: because agents are stored here by smart references
	// for some agents this operation will lead only to reference
	// counter descrement. Not to deletion of agent.
	m_agent_array.clear();
}

void
agent_coop_t::destroy( agent_coop_t * coop )
{
	delete coop;
}

agent_coop_t::agent_coop_t(
	const nonempty_name_t & name,
	disp_binder_unique_ptr_t coop_disp_binder,
	so_environment_t & env )
	:	m_coop_name( name.query_name() )
	,	m_coop_disp_binder( std::move(coop_disp_binder) )
	,	m_env( env )
	,	m_reference_count( 0 )
	,	m_parent_coop_ptr( nullptr )
	,	m_registration_status( COOP_NOT_REGISTERED )
{
}

const std::string &
agent_coop_t::query_coop_name() const
{
	return m_coop_name;
}

bool
agent_coop_t::has_parent_coop() const
{
	return !m_parent_coop_name.empty();
}

void
agent_coop_t::set_parent_coop_name(
	const nonempty_name_t & name )
{
	m_parent_coop_name = name.query_name();
}

const std::string &
agent_coop_t::parent_coop_name() const
{
	if( !has_parent_coop() )
		SO_5_THROW_EXCEPTION(
				rc_coop_has_no_parent,
				query_coop_name() + ": cooperation has no parent cooperation" );

	return m_parent_coop_name;
}

namespace
{
	/*!
	 * \since v.5.2.3
	 * \brief Helper function for notificator addition.
	 */
	inline void
	do_add_notificator_to(
		coop_notificators_container_ref_t & to,
		const coop_notificator_t & notificator )
	{
		if( !to )
		{
			to = coop_notificators_container_ref_t(
					new coop_notificators_container_t() );
		}

		to->add( notificator );
	}

} /* namespace anonymous */

void
agent_coop_t::add_reg_notificator(
	const coop_notificator_t & notificator )
{
	do_add_notificator_to( m_reg_notificators, notificator );
}

void
agent_coop_t::add_dereg_notificator(
	const coop_notificator_t & notificator )
{
	do_add_notificator_to( m_dereg_notificators, notificator );
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
agent_coop_t::do_registration_specific_actions(
	agent_coop_t * parent_coop )
{
	bind_agents_to_coop();
	define_all_agents();

	bind_agents_to_disp();

	start_all_agents();

	m_parent_coop_ptr = parent_coop;
	if( m_parent_coop_ptr )
		// Parent coop should known about existence of that coop.
		m_parent_coop_ptr->m_reference_count += 1;

	// Cooperation should assume that it is registered now.
	m_registration_status = COOP_REGISTERED;
}

void
agent_coop_t::do_deregistration_specific_actions()
{
	shutdown_all_agents();
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

	for(; it != it_end; ++it )
	{
		it->m_agent_ref->define_agent();
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
				m_env.so_environment_impl(), it->m_agent_ref );
		}
	}
	catch( const std::exception & ex )
	{
		unbind_agents_from_disp( it );

		throw;
	}
}

inline void
agent_coop_t::unbind_agents_from_disp(
	agent_array_t::iterator it )
{
	for( auto it_begin = m_agent_array.begin(); it != it_begin; )
	{
		--it;
		it->m_binder->unbind_agent(
			m_env.so_environment_impl(), it->m_agent_ref );
	}
}

void
agent_coop_t::start_all_agents()
{
	try
	{
		for( auto it = m_agent_array.begin(); it != m_agent_array.end(); ++it )
		{
			it->m_agent_ref->start_agent();
		}
	}
	catch( const std::exception & x )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT(
					 	"Exception during starting cooperation agents. "
						"Work cannot be continued. Cooperation: '%s'. "
						"Exception: %s" ),
						m_coop_name.c_str(),
						x.what() ) );

		ACE_OS::abort();
	}
}

void
agent_coop_t::shutdown_all_agents()
{
	try
	{
		for( auto it = m_agent_array.begin(); it != m_agent_array.end(); ++it )
		{
			it->m_agent_ref->shutdown_agent();
		}
	}
	catch( const std::exception & x )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				 SO_5_LOG_FMT(
					 	"Exception during shutting cooperation agents down. "
						"Work cannot be continued. Cooperation: '%s'. "
						"Exception: %s" ),
						m_coop_name.c_str(),
						x.what() ) );

		ACE_OS::abort();
	}
}

void
agent_coop_t::increment_usage_count()
{
	m_reference_count += 1;
}

void
agent_coop_t::decrement_usage_count()
{
	// If it is the last working agent then Environment should be
	// informed that the cooperation is ready to be deregistered.
	if( 0 == --m_reference_count )
	{
		// NOTE: usage counter incremented and decremented during
		// registration process even if registration of cooperation failed.
		// So decrement_usage_count() could be called when cooperation
		// has COOP_NOT_REGISTERED status.
		if( COOP_REGISTERED == m_registration_status )
		{
			m_registration_status = COOP_DEREGISTERING;
			m_env.so_environment_impl().ready_to_deregister_notify( this );
		}
	}
}

void
agent_coop_t::final_deregister_coop()
{
	unbind_agents_from_disp( m_agent_array.end() );

	m_env.so_environment_impl().final_deregister_coop( m_coop_name );
}

agent_coop_t *
agent_coop_t::parent_coop_ptr() const
{
	return m_parent_coop_ptr;
}

coop_notificators_container_ref_t
agent_coop_t::reg_notificators() const
{
	return m_reg_notificators;
}

coop_notificators_container_ref_t
agent_coop_t::dereg_notificators() const
{
	return m_dereg_notificators;
}

} /* namespace rt */

} /* namespace so_5 */

