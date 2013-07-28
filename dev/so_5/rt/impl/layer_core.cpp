/*
	SObjectizer 5.
*/

#include <algorithm>
#include <iterator>

#include <ace/Guard_T.h>

#include <cpp_util_2/h/lexcast.hpp>

#include <so_5/util/h/apply_throwing_strategy.hpp>
#include <so_5/rt/impl/h/layer_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// typed_layer_ref_t
//

typed_layer_ref_t::typed_layer_ref_t()
	:
		m_true_type( quick_typeid_t< int >::m_type_wrapper )
{
}

typed_layer_ref_t::typed_layer_ref_t(
	const so_layer_map_t::value_type & v )
	:
		m_true_type( v.first ),
		m_layer( v.second )
{
}

typed_layer_ref_t::typed_layer_ref_t(
	const type_wrapper_t & type,
	const so_layer_ref_t & layer )
	:
		m_true_type( type ),
		m_layer( layer )
{
}

bool
typed_layer_ref_t::operator < ( const typed_layer_ref_t & tl ) const
{
	return m_true_type < tl.m_true_type;
}

//
// layer_core_t
//

layer_core_t::layer_core_t(
	const so_layer_map_t & so_layers,
	so_environment_t * env )
	:
		m_env( env ),
		m_default_layers( so_layers.begin(), so_layers.end() )
{
	for( so_layer_list_t::iterator
		it = m_default_layers.begin(),
		it_end = m_default_layers.end();
		it != it_end;
		++it )
	{
		it->m_layer->bind_to_environment( m_env );
	}
}

layer_core_t::~layer_core_t()
{
}

//! ѕоиск сло€ по массиву.
inline so_layer_list_t::const_iterator
search_for_layer(
	const so_layer_list_t & layers,
	const type_wrapper_t & type )
{
	so_layer_list_t::const_iterator
		it = layers.begin(),
		it_end = layers.end();

	for(
		size_t size = std::distance( it, it_end );
		size > 0;
		size = std::distance( it, it_end ) )
	{
		if( size <= 8 )
		{
			// ≈сли осталось мало слоев дл€ поиска,
			// то ищем перебором.
			while( it != it_end )
			{
				if( type == it->m_true_type )
					return it;

				++it;
			}
		}
		else
		{
			// ≈сли слоев много, то бинарный поиск.
			so_layer_list_t::const_iterator it_center =
				it + size / 2;

			if( type == it_center->m_true_type )
				return it_center;

			if( type < it_center->m_true_type )
				it_end = it_center;
			else
				it = it_center + 1;
		}
	}

	return layers.end();
}

so_layer_t *
layer_core_t::query_layer(
	const type_wrapper_t & type ) const
{
	// —начала делаем поиск по сло€м по умолчанию.
	so_layer_list_t::const_iterator layer_it = search_for_layer(
		m_default_layers,
		type );

	if( m_default_layers.end() != layer_it )
		return layer_it->m_layer.get();

	ACE_Read_Guard< ACE_RW_Thread_Mutex > lock( m_extra_layers_lock );
	// ≈сли слой не найден, то
	// пытаемс€ найти его среди дополнительных слоев.
	layer_it = search_for_layer(
		m_extra_layers,
		type );

	if( m_extra_layers.end() != layer_it )
		return layer_it->m_layer.get();

	return nullptr;
}

ret_code_t
layer_core_t::start()
{
	so_layer_list_t::iterator
		it = m_default_layers.begin(),
		it_end = m_default_layers.end();

	for(; it != it_end; ++it )
	{
		const ret_code_t rc = it->m_layer->start();

		if( rc )
		{
			so_layer_list_t::iterator it_stoper = m_default_layers.begin();
			for(; it_stoper != it; ++it_stoper )
				it_stoper->m_layer->shutdown();

			for(it_stoper = m_default_layers.begin(); it_stoper != it; ++it_stoper )
				it_stoper->m_layer->wait();

			return rc;
		}
	}

	return 0;
}

void
call_shutdown( typed_layer_ref_t &  tl )
{
	tl.m_layer->shutdown();
}
void
call_wait( typed_layer_ref_t &  tl )
{
	tl.m_layer->wait();
}

void
layer_core_t::shutdown_extra_layers()
{
	std::for_each(
		m_extra_layers.begin(),
		m_extra_layers.end(),
		call_shutdown );
}

void
layer_core_t::wait_extra_layers()
{
	std::for_each(
		m_extra_layers.begin(),
		m_extra_layers.end(),
		call_wait );

	m_extra_layers.clear();
}

void
layer_core_t::shutdown_default_layers()
{
	std::for_each(
		m_default_layers.begin(),
		m_default_layers.end(),
		call_shutdown );
}

void
layer_core_t::wait_default_layers()
{
	std::for_each(
		m_default_layers.begin(),
		m_default_layers.end(),
		call_wait );

}

ret_code_t
layer_core_t::add_extra_layer(
	const type_wrapper_t & type,
	const so_layer_ref_t & layer,
	throwing_strategy_t throwing_strategy )
{
	if( nullptr == layer.get() )
		return so_5::util::apply_throwing_strategy(
			rc_trying_to_add_nullptr_extra_layer,
			throwing_strategy,
			"trying to add nullptr extra layer" );

	if( m_default_layers.end() != search_for_layer( m_default_layers, type ) )
		return so_5::util::apply_throwing_strategy(
			rc_trying_to_add_extra_layer_that_already_exists_in_default_list,
			throwing_strategy,
			"trying to add extra layer that already exists in default list" );

	//  огда ищем по дополнительным сло€м
	// уже надо ставить защиту.
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_extra_layers_lock );

	if( m_extra_layers.end() != search_for_layer( m_extra_layers, type ) )
		return so_5::util::apply_throwing_strategy(
			rc_trying_to_add_extra_layer_that_already_exists_in_extra_list,
			throwing_strategy,
			"trying to add extra layer that already exists in extra list" );

	// “акого сло€ нет, тогда добавл€ем прив€зываем его к
	// so_environment.
	layer->bind_to_environment( m_env );

	const ret_code_t rc = layer->start();
	if( rc )
		return so_5::util::apply_throwing_strategy(
			rc_unable_to_start_extra_layer,
			throwing_strategy,
			"unable to start extra layer rc: " + cpp_util_2::slexcast( rc ) );

	typed_layer_ref_t typed_layer( type, layer );

	m_extra_layers.insert(
		std::lower_bound(
			m_extra_layers.begin(),
			m_extra_layers.end(),
			typed_layer ),
		typed_layer );

	return 0;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
