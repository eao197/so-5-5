/*
	SObjectizer 5.
*/

#include <algorithm>

#include <so_5/h/exception.hpp>

#include <so_5/rt/impl/h/local_mbox.hpp>
#include <so_5/rt/impl/h/named_local_mbox.hpp>
#include <so_5/rt/impl/h/mpsc_mbox.hpp>
#include <so_5/rt/impl/h/mbox_core.hpp>
#include <so_5/rt/impl/h/mchain_details.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// mbox_core_t
//

mbox_core_t::mbox_core_t(
	so_5::msg_tracing::tracer_t * tracer )
	:	m_tracer{ tracer }
	,	m_mbox_id_counter{ 1 }
{
}

mbox_core_t::~mbox_core_t()
{
}


mbox_t
mbox_core_t::create_local_mbox()
{
	auto id = ++m_mbox_id_counter;
	if( !m_tracer )
		return mbox_t{ new local_mbox_without_tracing{ id } };
	else
		return mbox_t{ new local_mbox_with_tracing{ id, *m_tracer } };
}

mbox_t
mbox_core_t::create_local_mbox(
	const nonempty_name_t & mbox_name )
{
	return create_named_mbox(
			mbox_name,
			[this]() { return create_local_mbox(); } );
}

mbox_t
mbox_core_t::create_mpsc_mbox(
	agent_t * single_consumer,
	const so_5::rt::message_limit::impl::info_storage_t * limits_storage )
{
	const auto id = ++m_mbox_id_counter;

	std::unique_ptr< abstract_message_box_t > actual_mbox;
	if( limits_storage )
	{
		if( !m_tracer )
			actual_mbox.reset( new limitful_mpsc_mbox_without_tracing{
					id,
					single_consumer,
					*limits_storage } );
		else
			actual_mbox.reset( new limitful_mpsc_mbox_with_tracing{
					id,
					single_consumer,
					*limits_storage,
					*m_tracer } );
	}
	else
	{
		if( !m_tracer )
			actual_mbox.reset( new limitless_mpsc_mbox_without_tracing{
					id,
					single_consumer } );
		else
			actual_mbox.reset( new limitless_mpsc_mbox_with_tracing{
					id,
					single_consumer,
					*m_tracer } );
	}

	return mbox_t{ actual_mbox.release() };
}

void
mbox_core_t::destroy_mbox(
	const std::string & name )
{
	std::lock_guard< std::mutex > lock( m_dictionary_lock );

	named_mboxes_dictionary_t::iterator it =
		m_named_mboxes_dictionary.find( name );

	if( m_named_mboxes_dictionary.end() != it )
	{
		const unsigned int ref_count = --(it->second.m_external_ref_count);
		if( 0 == ref_count )
			m_named_mboxes_dictionary.erase( it );
	}
}

namespace {

template< typename Q, typename... A >
mchain
make_mchain(
	so_5::msg_tracing::tracer_t * tracer,
	A &&... args )
	{
		using namespace so_5::mchain_props;
		using namespace so_5::rt::impl::msg_tracing_helpers;
		using D = mchain_tracing_disabled_base;
		using E = mchain_tracing_enabled_base;

		if( tracer )
			return mchain{
					new mchain_template< Q, E >{
						std::forward<A>(args)...,
						*tracer } };
		else
			return mchain{
					new mchain_template< Q, D >{
						std::forward<A>(args)... } };
	}

} /* namespace anonymous */

mchain
mbox_core_t::create_mchain(
	environment_t & env,
	const mchain_params & params )
{
	using namespace so_5::mchain_props;
	using namespace so_5::mchain_props::details;

	auto id = ++m_mbox_id_counter;

	if( params.capacity().unlimited() )
		return make_mchain< unlimited_demand_queue >(
				m_tracer,
				env,
				id,
				params.capacity() );
	else if( storage_memory::dynamic == params.capacity().memory() )
		return make_mchain< limited_dynamic_demand_queue >(
				m_tracer,
				env,
				id,
				params.capacity() );
	else
		return make_mchain< limited_preallocated_demand_queue >(
				m_tracer,
				env,
				id,
				params.capacity() );
}

mbox_core_stats_t
mbox_core_t::query_stats()
{
	std::lock_guard< std::mutex > lock{ m_dictionary_lock };

	return mbox_core_stats_t{ m_named_mboxes_dictionary.size() };
}

mbox_t
mbox_core_t::create_named_mbox(
	const nonempty_name_t & nonempty_name,
	const std::function< mbox_t() > & factory )
{
	const std::string & name = nonempty_name.query_name();
	std::lock_guard< std::mutex > lock( m_dictionary_lock );

	named_mboxes_dictionary_t::iterator it =
		m_named_mboxes_dictionary.find( name );

	if( m_named_mboxes_dictionary.end() != it )
	{
		++(it->second.m_external_ref_count);
		return mbox_t(
			new named_local_mbox_t(
				name,
				it->second.m_mbox,
				*this ) );
	}

	// There is no mbox with such name. New mbox should be created.
	mbox_t mbox_ref = factory();

	m_named_mboxes_dictionary[ name ] = named_mbox_info_t( mbox_ref );

	return mbox_t( new named_local_mbox_t( name, mbox_ref, *this ) );
}

//
// mbox_core_ref_t
//

mbox_core_ref_t::mbox_core_ref_t()
	:
		m_mbox_core_ptr( nullptr )
{
}

mbox_core_ref_t::mbox_core_ref_t(
	mbox_core_t * mbox_core )
	:
		m_mbox_core_ptr( mbox_core )
{
	inc_mbox_core_ref_count();
}

mbox_core_ref_t::mbox_core_ref_t(
	const mbox_core_ref_t & mbox_core_ref )
	:
		m_mbox_core_ptr( mbox_core_ref.m_mbox_core_ptr )
{
	inc_mbox_core_ref_count();
}

void
mbox_core_ref_t::operator = (
	const mbox_core_ref_t & mbox_core_ref )
{
	if( &mbox_core_ref != this )
	{
		dec_mbox_core_ref_count();

		m_mbox_core_ptr = mbox_core_ref.m_mbox_core_ptr;
		inc_mbox_core_ref_count();
	}

}

mbox_core_ref_t::~mbox_core_ref_t()
{
	dec_mbox_core_ref_count();
}

inline void
mbox_core_ref_t::dec_mbox_core_ref_count()
{
	if( m_mbox_core_ptr &&
		0 == m_mbox_core_ptr->dec_ref_count() )
	{
		delete m_mbox_core_ptr;
		m_mbox_core_ptr = nullptr;
	}
}

inline void
mbox_core_ref_t::inc_mbox_core_ref_count()
{
	if( m_mbox_core_ptr )
		m_mbox_core_ptr->inc_ref_count();
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
