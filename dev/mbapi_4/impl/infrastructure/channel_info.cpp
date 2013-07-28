/*
	MBAPI 4.
*/

#include <mbapi_4/impl/infrastructure/h/channel_info.hpp>

namespace mbapi_4
{

namespace impl
{

namespace infrastructure
{

//
// channel_info_t
//

channel_info_t::channel_info_t()
{
}

channel_info_t::channel_info_t(
	const mbapi_node_uid_t & mbapi_node_uid,
	const so_5::rt::mbox_ref_t & mbox )
	:
		m_node_uid( mbapi_node_uid ),
		m_mbox( mbox )
{
}

//! Вспомогательный класс для поиска исчезнувших точек.
struct points_vector_modification_helper
{
	template < class POINT_TABLE, class POINT_LIST >
	static void
	find_dissapered(
		const POINT_TABLE & point_table,
		POINT_LIST & points,
		POINT_LIST & dissapered_points )
	{
		std::vector< bool > exist_flags( points.size(), false );

		for(
			size_t i = 0, size = points.size();
			i < size;
			++i )
		{
			const bool exists =
				point_table.end() !=
					point_table.find( points[ i ] );

			exist_flags[ i ] = exists;
			if( !exists )
				dissapered_points.push_back( points[ i ] );
		}

		if( !dissapered_points.empty() )
		{
			size_t insert_index = 0;
			for(
				size_t i = 0, size = points.size();
				i < size;
				++i )
			{
				if( exist_flags[ i ] )
					points[ insert_index++ ] = points[ i ];
			}

			points.resize( insert_index );
		}
	}

	template < class POINT, class POINT_LIST >
	static void
	insert(
		const POINT & point,
		POINT_LIST & points )
	{
		typename POINT_LIST::iterator it =
			std::find( points.begin(), points.end(), point );

		if( points.end() == it )
			points.push_back( point );
	}

	template < class POINT, class POINT_LIST >
	static void
	remove(
		const POINT & point,
		POINT_LIST & points )
	{
		typename POINT_LIST::iterator it =
			std::find( points.begin(), points.end(), point );

		if( points.end() != it )
			points.erase( it );
	}
};

void
channel_info_t::find_dissapered_endpoints(
	const available_endpoint_table_t & endpoint_table,
	endpoint_list_t & dissapered_endpoints )
{
	points_vector_modification_helper::find_dissapered(
		endpoint_table,
		m_endpoints,
		dissapered_endpoints );
}

void
channel_info_t::find_dissapered_stagepoints(
	const available_stagepoint_table_t & stagepoint_table,
	stagepoint_list_t & dissapered_stagepoints )
{
	points_vector_modification_helper::find_dissapered(
		stagepoint_table,
		m_stagepoints,
		dissapered_stagepoints );
}



void
channel_info_t::insert_endpoint(
	const endpoint_t & endpoint )
{
	points_vector_modification_helper::insert( endpoint, m_endpoints );
}

void
channel_info_t::remove_endpoint(
	const endpoint_t & endpoint )
{
	points_vector_modification_helper::remove( endpoint, m_endpoints );
}

void
channel_info_t::insert_stagepoint(
	const stagepoint_t & stagepoint )
{
	points_vector_modification_helper::insert( stagepoint, m_stagepoints );
}

void
channel_info_t::remove_stagepoint(
	const stagepoint_t & stagepoint )
{
	points_vector_modification_helper::remove( stagepoint, m_stagepoints );
}

} /* namespace infrastructure */

} /* namespace impl */

} /* namespace mbapi_4 */
