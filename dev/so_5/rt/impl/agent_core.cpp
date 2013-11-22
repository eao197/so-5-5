/*
	SObjectizer 5.
*/

#include <ace/Guard_T.h>

#include <so_5/h/log_err.hpp>

#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/agent_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

namespace agent_core_details
{

/*!
 * \since v.5.2.3
 * \brief Helper class for doing all actions related to
 * start of cooperation deregistration.
 *
 * This class is necessary because addition of parent-child relationship
 * in version 5.2.3. And since that version deregistration of cooperation
 * is more complex process then in previous versions.
 *
 * \attention On some stages of deregistration an exception leads to
 * call to abort().
 */
class deregistration_processor_t
{
public :
	//! Constructor.
	deregistration_processor_t(
		//! Owner of all data.
		agent_core_t & core,
		//! Name of root cooperation to be deregistered.
		const std::string & root_coop_name );

	//! Do all necessary actions.
	void
	process();

private :
	//! Owner of all data to be handled.
	agent_core_t & m_core;

	//! Name of root cooperation to be deregistered.
	const std::string & m_root_coop_name;

	//! Cooperations to be deregistered.
	std::vector< agent_coop_ref_t > m_coops_to_dereg;

	//! Names of cooperations to be deregistered.
	std::vector< std::string > m_coops_names_to_process;

	void
	first_stage();

	bool
	has_something_to_deregister() const;

	void
	second_stage();

	agent_coop_ref_t
	ensure_root_coop_exists() const;

	void
	collect_and_modity_coop_info(
		const agent_coop_ref_t & root_coop );

	void
	collect_coops();

	void
	modify_registered_and_deregistered_maps();

	void
	initiate_abort_on_exception(
		const std::exception & x );
};

deregistration_processor_t::deregistration_processor_t(
	agent_core_t & core,
	const std::string & root_coop_name )
	:	m_core( core )
	,	m_root_coop_name( root_coop_name )
{}

void
deregistration_processor_t::process()
{
	first_stage();

	if( has_something_to_deregister() )
		second_stage();
}

void
deregistration_processor_t::first_stage()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_core.m_coop_operations_lock );

	if( m_core.m_deregistered_coop.end() ==
			m_core.m_deregistered_coop.find( m_root_coop_name ) )
	{
		agent_coop_ref_t coop = ensure_root_coop_exists();

		collect_and_modity_coop_info( coop );
	}
}

bool
deregistration_processor_t::has_something_to_deregister() const
{
	return !m_coops_to_dereg.empty();
}

void
deregistration_processor_t::second_stage()
{
	// Exceptions must lead to abort at this deregistration stage.
	try
	{
		// All cooperations should start deregistration actions.
		std::for_each(
				m_coops_to_dereg.begin(),
				m_coops_to_dereg.end(),
				[]( const agent_coop_ref_t & coop ) {
					agent_coop_private_iface_t::undefine_all_agents( *coop );
				} );
	}
	catch( const std::exception & x )
	{
		initiate_abort_on_exception( x );
	}
}

agent_coop_ref_t
deregistration_processor_t::ensure_root_coop_exists() const
{
	// It is an error if the cooperation is not registered.
	auto it = m_core.m_registered_coop.find( m_root_coop_name );

	if( m_core.m_registered_coop.end() == it )
	{
		SO_5_THROW_EXCEPTION(
			rc_coop_has_not_found_among_registered_coop,
			"coop with name '" + m_root_coop_name +
			"' not found among registered cooperations" );
	}

	return it->second;
}

void
deregistration_processor_t::collect_and_modity_coop_info(
	const agent_coop_ref_t & root_coop )
{
	// Exceptions must lead to abort at this deregistration stage.
	try
	{
		m_coops_to_dereg.push_back( root_coop );
		m_coops_names_to_process.push_back( m_root_coop_name );

		collect_coops();

		modify_registered_and_deregistered_maps();
	}
	catch( const std::exception & x )
	{
		initiate_abort_on_exception( x );
	}
}

void
deregistration_processor_t::collect_coops()
{
	for( auto n = m_coops_names_to_process.begin();
			n != m_coops_names_to_process.end(); ++n )
	{
		const agent_core_t::parent_child_coop_names_t relation(
				*n, std::string() );

		for( auto f = m_core.m_parent_child_relations.lower_bound( relation );
				f != m_core.m_parent_child_relations.end() && f->first == *n;
				++f )
		{
			auto it = m_core.m_registered_coop.find( f->second );
			if( it != m_core.m_registered_coop.end() )
			{
				m_coops_to_dereg.push_back( it->second );
				m_coops_names_to_process.push_back( it->first );
			}
			else
			{
				SO_5_THROW_EXCEPTION(
						rc_unexpected_error,
						f->second + ": cooperation not registered, but "
							"declared as child for: '" + f->first + "'" );
			}
		}
	}
}

void
deregistration_processor_t::modify_registered_and_deregistered_maps()
{
	std::for_each(
			m_coops_names_to_process.begin(),
			m_coops_names_to_process.end(),
			[this]( const std::string & n ) {
				auto it = m_core.m_registered_coop.find( n );
				m_core.m_deregistered_coop.insert( *it );
				m_core.m_registered_coop.erase( it );
			} );
}

void
deregistration_processor_t::initiate_abort_on_exception(
	const std::exception & x )
{
	ACE_ERROR(
			(LM_EMERGENCY,
			 SO_5_LOG_FMT( "Exception during cooperation deregistration. "
					"Work cannot be continued. Cooperation: '%s'. "
					"Exception: %s" ),
					m_root_coop_name.c_str(),
					x.what() ) );

	ACE_OS::abort();
}

} /* namespace agent_core_details */

agent_core_t::agent_core_t(
	so_environment_t & so_environment,
	unsigned int agent_queue_mutex_pool_size,
	coop_listener_unique_ptr_t coop_listener )
	:
		m_so_environment( so_environment ),
		m_agent_queue_mutex_pool( agent_queue_mutex_pool_size ),
		m_deregistration_started_cond( m_coop_operations_lock ),
		m_deregistration_finished_cond( m_coop_operations_lock ),
		m_deregistration_started( false ),
		m_coop_listener( std::move( coop_listener ) )
{
}

agent_core_t::~agent_core_t()
{
}

void
agent_core_t::start()
{
	m_deregistration_started = false;
	m_coop_dereg_executor.start();
}

void
agent_core_t::finish()
{
	// Deregistration of all cooperations should be initiated.
	deregister_all_coop();

	// Deregistration of all cooperations should be finished.
	wait_all_coop_to_deregister();

	// Notify a dedicated thread and wait while it will be stopped.
	m_coop_dereg_executor.finish();
}

local_event_queue_unique_ptr_t
agent_core_t::create_local_queue()
{
	local_event_queue_unique_ptr_t local_event_queue(
		new local_event_queue_t( m_agent_queue_mutex_pool ) );

	return local_event_queue;
}

void
agent_core_t::register_coop(
	agent_coop_unique_ptr_t agent_coop )
{
	if( 0 == agent_coop.get() )
		SO_5_THROW_EXCEPTION(
			rc_zero_ptr_to_coop,
			"zero ptr to coop passed" );

	const std::string coop_name = agent_coop->query_coop_name();

	try
	{
		// All the following actions should be taken under the lock.
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		// Name should be unique.
		ensure_new_coop_name_unique( coop_name );
		// Process parent coop.
		agent_coop_t * parent = find_parent_coop_if_necessary( *agent_coop );

		next_coop_reg_step__update_registered_coop_map(
				std::move(agent_coop),
				parent );
	}
	catch( const exception_t & ex )
	{
		throw;
	}
	catch( const std::exception & ex )
	{
		SO_5_THROW_EXCEPTION(
			rc_coop_define_agent_failed,
			ex.what() );
	}

	if( m_coop_listener.get() )
		m_coop_listener->on_registered( m_so_environment, coop_name );
}

void
agent_core_t::deregister_coop(
	const nonempty_name_t & name )
{
	agent_core_details::deregistration_processor_t processor(
			*this,
			name.query_name() );

	processor.process();
}

void
agent_core_t::ready_to_deregister_notify(
	agent_coop_t * coop )
{
	m_coop_dereg_executor.push_dereg_demand( coop );
}

void
agent_core_t::final_deregister_coop(
	const std::string coop_name )
{
	bool need_signal_dereg_finished;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		finaly_remove_cooperation_info( coop_name );

		// If we are inside shutdown process and this is the last
		// cooperation then a special flag should be set.
		need_signal_dereg_finished =
			m_deregistration_started && m_deregistered_coop.empty();
	}

	if( need_signal_dereg_finished )
		m_deregistration_finished_cond.signal();

	if( m_coop_listener.get() )
		m_coop_listener->on_deregistered(
			m_so_environment,
			coop_name );
}

void
agent_core_t::start_deregistration()
{
	bool signal_deregistration_started = false;
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

		if( !m_deregistration_started )
		{
			m_deregistration_started = true;
			signal_deregistration_started = true;
		}
	}

	if( signal_deregistration_started )
		m_deregistration_started_cond.signal();
}

void
agent_core_t::wait_for_start_deregistration()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	if( !m_deregistration_started )
		m_deregistration_started_cond.wait();
}

void
agent_core_t::coop_undefine_all_agents(
	agent_core_t::coop_map_t::value_type & coop )
{
	coop.second->undefine_all_agents();
}

void
agent_core_t::deregister_all_coop()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	std::for_each(
		m_registered_coop.begin(),
		m_registered_coop.end(),
		agent_core_t::coop_undefine_all_agents );

	m_deregistered_coop.insert(
		m_registered_coop.begin(),
		m_registered_coop.end() );

	m_registered_coop.clear();
	m_deregistration_started = true;
}

void
agent_core_t::wait_all_coop_to_deregister()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_coop_operations_lock );

	// Must wait for a signal is there are cooperations in
	// the deregistration process.
	if( !m_deregistered_coop.empty() )
	{
		// Wait for the deregistration finish.
		m_deregistration_finished_cond.wait();
	}
}

void
agent_core_t::ensure_new_coop_name_unique(
	const std::string & coop_name ) const
{
	if( m_registered_coop.end() != m_registered_coop.find( coop_name ) ||
		m_deregistered_coop.end() != m_deregistered_coop.find( coop_name ) )
	{
		SO_5_THROW_EXCEPTION(
			rc_coop_with_specified_name_is_already_registered,
			"coop with name \"" + coop_name + "\" is already registered" );
	}
}

agent_coop_t *
agent_core_t::find_parent_coop_if_necessary(
	const agent_coop_t & coop_to_be_registered ) const
{
	if( coop_to_be_registered.has_parent_coop() )
	{
		auto it = m_registered_coop.find(
				coop_to_be_registered.parent_coop_name() );
		if( m_registered_coop.end() != it )
		{
			SO_5_THROW_EXCEPTION(
				rc_parent_coop_not_found,
				"parent coop with name \"" +
					coop_to_be_registered.parent_coop_name() +
					"\" is not registered" );
		}

		return it->second.get();
	}

	return nullptr;
}

void
agent_core_t::next_coop_reg_step__update_registered_coop_map(
	agent_coop_unique_ptr_t coop,
	agent_coop_t * parent_coop_ptr )
{
	agent_coop_ref_t coop_ref( coop.release() );

	m_registered_coop[ coop_ref->query_coop_name() ] = coop_ref;

	// In case of error cooperation info should be removed
	// from m_registered_coop.
	try
	{
		next_coop_reg_step__parent_child_relation( coop_ref, parent_coop_ptr );
	}
	catch( const std::exception & )
	{
		m_registered_coop.erase( coop_ref->query_coop_name() );

		throw;
	}
}

void
agent_core_t::next_coop_reg_step__parent_child_relation(
	const agent_coop_ref_t & coop_ref,
	agent_coop_t * parent_coop_ptr )
{
	if( parent_coop_ptr )
	{
		m_parent_child_relations.insert(
			parent_child_coop_names_t(
				parent_coop_ptr->query_coop_name(),
				coop_ref->query_coop_name() ) );
	}

	// In case of error cooperation relation info should be removed
	// from m_parent_child_relations.
	try
	{
		coop_ref->do_registration_specific_actions( parent_coop_ptr );
	}
	catch( const std::exception & )
	{
		if( parent_coop_ptr )
		{
			m_parent_child_relations.erase(
				parent_child_coop_names_t(
					parent_coop_ptr->query_coop_name(),
					coop_ref->query_coop_name() ) );
		}

		throw;
	}
}

void
agent_core_t::finaly_remove_cooperation_info(
	const std::string & coop_name )
{
	auto it = m_deregistered_coop.find( coop_name );
	if( it != m_deregistered_coop.end() )
	{
		agent_coop_t * parent =
				agent_coop_private_iface_t::parent_coop_ptr( *(it->second) );
		if( parent )
		{
			m_parent_child_relations.erase(
					parent_child_coop_names_t(
							parent->query_coop_name(),
							coop_name ) );

			parent->entity_finished();
		}

		m_deregistered_coop.erase( it );
	}
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
