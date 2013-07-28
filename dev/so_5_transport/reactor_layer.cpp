/*
	SObjectizer 5 Transport.
*/

#include <algorithm>

#include <so_5/h/log_err.hpp>

#include <so_5_transport/h/reactor_layer.hpp>

namespace so_5_transport
{

//
// reactor_layer_params_t
//

reactor_layer_params_t &
reactor_layer_params_t::add_named_reactor(
	const so_5::rt::nonempty_name_t & name,
	so_5_transport::ace::reactor_instance_unique_ptr_t reactor )
{
	if( reactor.get() )
	{
		m_named_reactor_map[ name.query_name() ] =
			so_5_transport::ace::reactor_instance_ref_t( reactor.release() );
	}

	return *this;
}

const  so_5_transport::ace::reactor_instance_map_t &
reactor_layer_params_t::query_named_reactor_map() const
{
	return m_named_reactor_map;
}

//
// reactor_layer_t
//

reactor_layer_t::reactor_layer_t()
{
	create_default_reactor();
}

reactor_layer_t::reactor_layer_t(
	const reactor_layer_params_t & params )
{
	create_default_reactor();
	create_named_reactors( params.query_named_reactor_map() );
}

reactor_layer_t::~reactor_layer_t()
{
}

void
reactor_layer_t::create_default_reactor()
{
	m_default_reactor = so_5_transport::ace::make_select_reactor();

	if( !m_default_reactor.get() )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "failed to create default reactor" ) ) );

		ACE_OS::abort();
	}
}

void
reactor_layer_t::create_named_reactors(
	const so_5_transport::ace::reactor_instance_map_t & named_reactors )
{
	if( !named_reactors.empty() )
	{
		m_reactors_map.reset( new reactor_instance_hash_map_t(
			std::max< size_t >( 2*named_reactors.size(), 64 ) ) );

		so_5_transport::ace::reactor_instance_map_t::const_iterator
			it = named_reactors.begin(),
			it_end = named_reactors.end();

		for(; it != it_end; ++it )
		{
			SO_5_ABORT_ON_ACE_ERROR(
				m_reactors_map->bind(
					it->first,
					it->second ) );
		}
	}
}

ACE_Reactor *
reactor_layer_t::query_default_reactor() const
{
	return m_default_reactor->reactor();
}

ACE_Reactor *
reactor_layer_t::query_named_reactor(
	const std::string & reactor_name ) const
{
	so_5_transport::ace::reactor_instance_ref_t ptr;

	if( m_reactors_map.get() )
	{
		if( -1 == m_reactors_map->find( reactor_name, ptr ) )
			return 0;
	}

	return ptr->reactor();
}

so_5::ret_code_t
reactor_layer_t::start()
{
	// Нужно запустить все реакторы. Если же запуск
	// какого-то из них не получится, то все ранее запущенные
	// реакторы нужно будет остановить.
	int rc;

	rc = m_default_reactor->start();
	if( rc )
	{
		ACE_ERROR( (
			LM_ERROR,
			SO_5_LOG_FMT( "unable to start default reactor; rc: %d" ), rc ) );

		return rc_default_reactor_start_failed;
	}

	if( m_reactors_map.get() )
	{
		reactor_instance_hash_map_t::iterator
			it = m_reactors_map->begin(),
			it_end = m_reactors_map->end();

		for(; it != it_end; ++it )
		{
			rc = it->item()->start();
			if( rc )
			{
				reactor_instance_hash_map_t::iterator
					it_to_stop = m_reactors_map->begin();
				for(; it_to_stop != it; ++it_to_stop )
					it_to_stop->item()->stop();

				m_default_reactor->stop();

				ACE_ERROR( (
				LM_ERROR,
					SO_5_LOG_FMT( "unable to start reactor: '%s'; rc: %d" ),
					it->key().c_str(),
					rc ) );

				return rc_named_reactor_start_failed;
			}
		}
	}

	return 0;
}

void
reactor_layer_t::shutdown()
{
	m_default_reactor->stop();

	if( m_reactors_map.get() )
	{
		reactor_instance_hash_map_t::iterator
			it = m_reactors_map->begin(),
			it_end = m_reactors_map->end();

		for(; it != it_end; ++it )
		{
			it->item()->stop();
		}
	}
}

void
reactor_layer_t::wait()
{
	m_default_reactor->wait();

	if( m_reactors_map.get() )
	{
		reactor_instance_hash_map_t::iterator
			it = m_reactors_map->begin(),
			it_end = m_reactors_map->end();

		for(; it != it_end; ++it )
		{
			it->item()->wait();
		}
	}
}

} /* namespace so_5_transport */
