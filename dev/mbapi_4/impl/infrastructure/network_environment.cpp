/*
	MBAPI 4.
*/

#include <algorithm>
#include <iterator>

#include <so_5/h/exception.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/impl/infrastructure/h/network_environment.hpp>

namespace mbapi_4
{

namespace impl
{

namespace infrastructure
{

//
// network_environment_t
//

network_environment_t::network_environment_t()
	:
		m_node_uid( mbapi_node_uid_t::create() )
{
}

void
network_environment_t::add_local_endpoint(
	const endpoint_stage_chain_t & endpoint_stage_chain )
{
	const endpoint_t & endpoint = endpoint_stage_chain.endpoint();

	available_endpoint_ptr_t available_endpoint(
		new available_endpoint_t );

	available_endpoint->m_node_uid = m_node_uid;
	available_endpoint->m_distance = 0;
	available_endpoint->m_endpoint_stage_chain = endpoint_stage_chain;

	available_endpoint_table_t::iterator it_endpoint =
		m_endpoints.find( endpoint );

	if( m_endpoints.end() == it_endpoint )
		m_endpoints[ endpoint ] = available_endpoint;
	else
	{
		const channel_uid_wrapper_t & current_endpoint_channel_id =
			it_endpoint->second->m_channel_id;

		// Если уже есть такой локальный endpoint, то заменить его нельзя.
		if( current_endpoint_channel_id.is_local() )
			throw so_5::exception_t(
				"duplicating endpoint on local node",
				rc_duplicating_endpoint_on_local_node );

		channel_info_t & current_endpoint_channel_info =
			find_channel( current_endpoint_channel_id );

		current_endpoint_channel_info.remove_endpoint( endpoint );

		it_endpoint->second = available_endpoint;
	}
}

void
network_environment_t::remove_local_endpoint(
	const endpoint_t & endpoint )
{
	available_endpoint_table_t::iterator it_endpoint =
		m_endpoints.find( endpoint );

	if( m_endpoints.end() != it_endpoint )
	{
		const channel_uid_wrapper_t & current_endpoint_channel_id =
			it_endpoint->second->m_channel_id;

		if( current_endpoint_channel_id.is_local() )
			m_endpoints.erase( it_endpoint );
	}
}

void
network_environment_t::add_local_stagepoint(
	const stagepoint_t & stagepoint )
{
	available_stagepoint_ptr_t available_stagepoint(
		new available_stagepoint_t );

	available_stagepoint->m_node_uid = m_node_uid;
	available_stagepoint->m_distance = 0;

	available_stagepoint_table_t::iterator it_stagepoint =
		m_stagepoints.find( stagepoint );

	if( m_stagepoints.end() == it_stagepoint )
		m_stagepoints[ stagepoint ] = available_stagepoint;
	else
	{
		const channel_uid_wrapper_t & current_endpoint_channel_id =
			it_stagepoint->second->m_channel_id;

		// Если уже есть такой локальный stagepoint, то заменить его нельзя.
		if( current_endpoint_channel_id.is_local() )
			throw so_5::exception_t(
				"duplicating stagepoint on local node",
				rc_duplicating_stagepoint_on_local_node );

		channel_info_t & current_stagepoint_channel_info =
			find_channel( current_endpoint_channel_id );

		current_stagepoint_channel_info.remove_stagepoint( stagepoint );

		it_stagepoint->second = available_stagepoint;
	}
}

void
network_environment_t::remove_local_stagepoint(
	const stagepoint_t & stagepoint )
{
	available_stagepoint_table_t::iterator it_stagepoint =
		m_stagepoints.find( stagepoint );

	if( m_stagepoints.end() != it_stagepoint )
	{
		const channel_uid_wrapper_t & current_stagepoint_channel_id =
			it_stagepoint->second->m_channel_id;

		if( current_stagepoint_channel_id.is_local() )
			m_stagepoints.erase( it_stagepoint );
	}
}

void
network_environment_t::update_channel(
	const channel_uid_wrapper_t & channel_id,
	const mbapi_node_uid_t & node_uid,
	const so_5::rt::mbox_ref_t & mbox,
	const available_endpoint_table_t & endpoints,
	const available_stagepoint_table_t & stagepoints )
{
	channel_info_t & channel_info = find_channel( channel_id, node_uid, mbox );

	{ // Удаление конечных точек, которые были закреплены за этим каналом.
		endpoint_list_t endpoints_to_be_removed;
		channel_info.find_dissapered_endpoints(
			endpoints,
			endpoints_to_be_removed );

		if( !endpoints_to_be_removed.empty() )
			remove_endpoints( endpoints_to_be_removed );
	}

	{ // Удаление точек-стадий, которые были закреплены за этим каналом.
		stagepoint_list_t stagepoints_to_be_removed;
		channel_info.find_dissapered_stagepoints(
			stagepoints,
			stagepoints_to_be_removed );

		if( !stagepoints_to_be_removed.empty() )
			remove_stagepoints( stagepoints_to_be_removed );
	}

	// Синхронизация таблиц:

	sync_endpoint_table(
		channel_id,
		channel_info,
		endpoints );

	sync_stagepoint_table(
		channel_id,
		channel_info,
		stagepoints );
}

void
network_environment_t::copy_tables_data(
	infrastructure::available_endpoint_table_t & endpoints,
	infrastructure::available_stagepoint_table_t & stagepoints ) const
{
	copy_endpoints_table( endpoints );
	copy_stagepoints_table( stagepoints );
}

void
network_environment_t::delete_channel(
	const channel_uid_wrapper_t & channel_id )
{
	channel_info_table_t::iterator it_channel_info =
		m_channels_infos.find( channel_id );

	if( m_channels_infos.end() != it_channel_info )
	{
		channel_info_t & channel_info = *(it_channel_info->second);

		for(
			endpoint_list_t::const_iterator
				it = channel_info.endpoints().begin(),
				it_end = channel_info.endpoints().end();
			it != it_end;
			++it )
		{
			m_endpoints.erase( *it );
		}

		for(
			stagepoint_list_t::const_iterator
				it = channel_info.stagepoints().begin(),
				it_end = channel_info.stagepoints().end();
			it != it_end;
			++it )
		{
			m_stagepoints.erase( *it );
		}

		m_channels_infos.erase( it_channel_info );
	}

}

endpoint_list_t
network_environment_t::query_endpoint_list() const
{
	endpoint_list_t result;
	std::transform(
		m_endpoints.begin(),
		m_endpoints.end(),
		std::back_inserter( result ),
		[]( const available_endpoint_table_t::value_type & v ){
			return v.first;
		} );

	return result;
}

bool
network_environment_t::shift_to_next_stage_direct(
	const endpoint_t & ep,
	stagepoint_t & current_stage ) const
{
	available_endpoint_table_t::const_iterator it = m_endpoints.find( ep );

	if( m_endpoints.end() == it )
		return false;

	const stage_list_t & stages = it->second->m_endpoint_stage_chain.stages();

	stage_list_t::const_iterator stage_it =
		std::find( stages.begin(), stages.end(), current_stage.name() );
	if( stages.end() == stage_it )
		return false;

	std::advance( stage_it, 1 );

	if( stages.end() != stage_it )
		current_stage.set_name( *stage_it );
	else
		current_stage.set_name( ep.name() );

	return true;
}

bool
network_environment_t::shift_to_first_stage_direct(
	const endpoint_t & ep,
	stagepoint_t & current_stage ) const
{
	available_endpoint_table_t::const_iterator it = m_endpoints.find( ep );

	if( m_endpoints.end() == it )
		return false;

	const stage_list_t & stages = it->second->m_endpoint_stage_chain.stages();

	if( stages.empty() )
		current_stage = stagepoint_t( ep.name(), ep.name() );
	else
		current_stage = stagepoint_t( stages.front(), ep.name() );

	return true;
}

bool
network_environment_t::shift_to_next_stage(
	const endpoint_t & from,
	const endpoint_t & to,
	stagepoint_t & current_stage ) const
{
	// Сначала смотрим является ли стадия частью конечной точки to.
	if( to.name() == current_stage.endpoint_name() )
	{
		// Если это уже сама конечная точка,
		// то следующей стадии нет.
		if( current_stage.is_endpoint() )
			return false;

		return shift_to_next_stage_direct( to, current_stage );
	}

	// Если стадия является частью from.
	if( from.name() == current_stage.endpoint_name() )
	{
		available_endpoint_table_t::const_iterator it = m_endpoints.find( from );

		if( m_endpoints.end() == it )
			return false;

		const stage_list_t & stages = it->second->m_endpoint_stage_chain.stages();

		// Если это конечная точка
		if( current_stage.is_endpoint() )
		{
			// Если это не сама конечная точка,
			// то раз цепочка стадий пуста, то следующую стадию надо искать
			// в конечной точке to.
			if( stages.empty() )
				return shift_to_first_stage_direct( to, current_stage );
			else
			{
				current_stage.set_name( stages.back() );
				return true;
			}
		}
		else
		{
			stage_list_t::const_reverse_iterator stage_it =
				std::find( stages.rbegin(), stages.rend(), current_stage.name() );

			if( stages.rend() != stage_it )
			{
				// Если такая стадия есть, то надо взять следующую.
				std::advance( stage_it, 1 );

				// Если есть следуюущая то это, то что надо.
				// иначе ищем первую стадию конечной точки to.
				if( stages.rend() != stage_it )
				{
					current_stage.set_name( *stage_it );
					return true;
				}
				else
					return shift_to_first_stage_direct( to, current_stage );
			}
		}
	}

	return false;
}

bool
network_environment_t::define_channel_info(
	const stagepoint_t & stagepoint,
	so_5::rt::mbox_ref_t & mbox,
	channel_uid_wrapper_t & channel_id ) const
{
	available_stagepoint_table_t::const_iterator it =
		m_stagepoints.find( stagepoint );

	if( m_stagepoints.end() != it )
	{
		// Определяем идентификатор канала.
		channel_id = it->second->m_channel_id;

		channel_info_table_t::const_iterator channel_it =
			m_channels_infos.find( channel_id );

		if( m_channels_infos.end() != channel_it )
		{
			// Определяем mbox.
			mbox = channel_it->second->mbox();
			return true;
		}
	}

	return false;
}

channel_info_t &
network_environment_t::find_channel(
	const channel_uid_wrapper_t & channel_id )
{
	channel_info_table_t::iterator it_channel_info =
		m_channels_infos.find( channel_id );

	return *(it_channel_info->second);
}

channel_info_t &
network_environment_t::find_channel(
	const channel_uid_wrapper_t & channel_id,
	const mbapi_node_uid_t & node_uid,
	const so_5::rt::mbox_ref_t & mbox )
{
	channel_info_table_t::iterator it_channel_info =
		m_channels_infos.find( channel_id );

	// Если канала не было то заводим информацию о нем.
	if( m_channels_infos.end() != it_channel_info )
		return *(it_channel_info->second);

	channel_info_ptr_t channel_info( new channel_info_t( node_uid, mbox ) );
	m_channels_infos[ channel_id ] = channel_info;

	return *channel_info;
}

void
network_environment_t::remove_endpoints(
	const endpoint_list_t & endpoints_to_be_removed )
{
	for(
		endpoint_list_t::const_iterator
			it = endpoints_to_be_removed.begin(),
			it_end = endpoints_to_be_removed.end();
		it != it_end;
		++it )
	{
		m_endpoints.erase( *it );
	}
}

void
network_environment_t::remove_stagepoints(
	const stagepoint_list_t & stagepoints_to_be_removed )
{
	for(
		stagepoint_list_t::const_iterator
			it = stagepoints_to_be_removed.begin(),
			it_end = stagepoints_to_be_removed.end();
		it != it_end;
		++it )
	{
		m_stagepoints.erase( *it );
	}
}

void
network_environment_t::sync_endpoint_table(
	const channel_uid_wrapper_t & channel_id,
	channel_info_t & channel_info,
	const available_endpoint_table_t & endpoints )
{
	for(
		available_endpoint_table_t::const_iterator
			it = endpoints.begin(),
			it_end = endpoints.end();
		it != it_end;
		++it )
	{
		available_endpoint_table_t::iterator it_endpoint =
			m_endpoints.find( it->first );

		available_endpoint_ptr_t available_endpoint(
			new available_endpoint_t( *(it->second) ) );
		available_endpoint->m_distance += 1;
		available_endpoint->m_channel_id = channel_id;

		// Если информация о такой конечной точке уже хранится,
		// то возможно ее нужно заменить или обновить.
		if( m_endpoints.end() != it_endpoint )
		{
			const channel_uid_wrapper_t & current_endpoint_channel_id =
				it_endpoint->second->m_channel_id;

			// Если расстояние до это endpoint через данный канал
			// меньше (1), или оно и было через этот карнал (2),
			// то надо заменить информацию о конечной точке,
			// потому что в случае (1) конечная точка ближе,
			// а в случае (2) ее параметры могли измениться.
			if( it_endpoint->second->m_distance > available_endpoint->m_distance ||
				current_endpoint_channel_id == channel_id )
			{
				// Если каналы разные, то этот маршрут оказался короче.
				// надо ищменить принадлежность точки каналу.
				if( current_endpoint_channel_id != channel_id )
				{
					channel_info_t & current_endpoint_channel_info =
						find_channel( current_endpoint_channel_id );

					current_endpoint_channel_info.remove_endpoint(
						it->first );

					channel_info.insert_endpoint( it->first );
				}

				it_endpoint->second = available_endpoint;
			}
		}
		else
		{
			// Если информация о такой конечной отсутствует,
			// то это значит, что она новая.
			m_endpoints[ it->first ] = available_endpoint;
			channel_info.insert_endpoint( it->first );
		}
	}
}

void
network_environment_t::sync_stagepoint_table(
	const channel_uid_wrapper_t & channel_id,
	channel_info_t & channel_info,
	const available_stagepoint_table_t & stagepoints )
{
	for(
		available_stagepoint_table_t::const_iterator
			it = stagepoints.begin(),
			it_end = stagepoints.end();
		it != it_end;
		++it )
	{
		available_stagepoint_table_t::iterator it_stagepoint =
			m_stagepoints.find( it->first );

		available_stagepoint_ptr_t available_stagepoint(
			new available_stagepoint_t( *(it->second) ) );
		available_stagepoint->m_distance += 1;
		available_stagepoint->m_channel_id = channel_id;

		// Если информация о такой конечной точке уже хранится,
		// то возможно ее нужно заменить или обновить.
		if( m_stagepoints.end() != it_stagepoint )
		{
			const channel_uid_wrapper_t & current_stagepoint_channel_id =
				it_stagepoint->second->m_channel_id;

			// Если расстояние до это stagepoint через данный канал
			// меньше (1), или оно и было через этот карнал (2),
			// то надо заменить информацию о конечной точке,
			// потому что в случае (1) конечная точка ближе,
			// а в случае (2) ее параметры могли измениться.
			if( it_stagepoint->second->m_distance > available_stagepoint->m_distance ||
				current_stagepoint_channel_id == channel_id )
			{
				// Если каналы разные, то этот маршрут оказался короче.
				// надо ищменить принадлежность точки каналу.
				if( current_stagepoint_channel_id != channel_id )
				{
					channel_info_t & current_stagepoint_channel_info =
						find_channel( current_stagepoint_channel_id );

					current_stagepoint_channel_info.remove_stagepoint(
						it->first );

					channel_info.insert_stagepoint( it->first );
				}

				it_stagepoint->second = available_stagepoint;
			}
		}
		else
		{
			// Если информация о такой конечной отсутствует,
			// то это значит, что она новая.
			m_stagepoints[ it->first ] = available_stagepoint;
			channel_info.insert_stagepoint( it->first );
		}
	}
}


void
network_environment_t::copy_endpoints_table(
	infrastructure::available_endpoint_table_t & endpoints ) const
{
	for(
		available_endpoint_table_t::const_iterator
			it = m_endpoints.begin(),
			it_end = m_endpoints.end();
		it != it_end;
		++it )
	{
		endpoints[ it->first ] = available_endpoint_ptr_t(
			new available_endpoint_t( *(it->second) ) );
	}
}

void
network_environment_t::copy_stagepoints_table(
	infrastructure::available_stagepoint_table_t & stagepoints ) const
{
	for(
		available_stagepoint_table_t::const_iterator
			it = m_stagepoints.begin(),
			it_end = m_stagepoints.end();
		it != it_end;
		++it )
	{
		stagepoints[ it->first ] = available_stagepoint_ptr_t(
			new available_stagepoint_t( *(it->second) ) );
	}
}

} /* namespace infrastructure */

} /* namespace impl */

} /* namespace mbapi_4 */
