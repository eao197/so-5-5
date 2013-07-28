/*
	MBAPI 4.
*/

#include <algorithm>

#include <so_5/h/exception.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>
#include <mbapi_4/defs/h/endpoint.hpp>

#include <mbapi_4/defs/impl/h/check_point_name.hpp>

namespace mbapi_4
{

//
// endpoint_t
//

endpoint_t::endpoint_t()
{
}

endpoint_t::endpoint_t(
	const std::string & endpoint_name )
	:
		m_endpoint_name( impl::check_point_name( endpoint_name ) )
{
}

void
endpoint_t::set_name(
	const std::string & endpoint_name )
{
	m_endpoint_name = impl::check_point_name( endpoint_name );
}

//
// endpoint_stage_chain_t
//

endpoint_stage_chain_t::endpoint_stage_chain_t()
{
}

endpoint_stage_chain_t::endpoint_stage_chain_t(
	const endpoint_t & endpoint )
	:
		m_endpoint( endpoint )
{
}

endpoint_stage_chain_t::endpoint_stage_chain_t(
	const endpoint_t & endpoint,
	const stage_list_t & stages )
	:
		m_endpoint( endpoint ),
		m_stages( stages )
{
	validate_stage_names();
}

endpoint_stage_chain_t::~endpoint_stage_chain_t()
{
}

void
endpoint_stage_chain_t::set_stages(
	const stage_list_t & stages )
{
	m_stages.assign( stages.begin(), stages.end() );
	validate_stage_names();
}

void
endpoint_stage_chain_t::validate_stage_names() const
{
	// ѕровер€ем, чтобы им€ конечной точки не совпадало с именами стадий.
	if( m_stages.end() !=
		std::find(
			m_stages.begin(),
			m_stages.end(),
			m_endpoint.name() ) )
	{
		throw so_5::exception_t(
			"stage name should not match endpoint name: " + m_endpoint.name(),
			rc_sp_name_matches_ep_name );
	}

	// ѕровер€ем, чтобы им€ имена стадий не повтор€лись.
	for(
		stage_list_t::const_iterator
			it = m_stages.begin(),
			it_end = m_stages.end();
		it != it_end;
		++it )
	{
		// ѕровер€ем само им€.
		impl::check_point_name( *it );

		// ≈сли находим совпадение, то это ошибка.
		if( it_end != find( it + 1, it_end, *it ) )
		{
			throw so_5::exception_t(
				"duplicating stage name: " + *it,
				rc_duplicating_stage_name );
		}
	}
}

namespace /* ananymous */
{

const std::string g_spaces( " \t" );

std::string
trimspaces(
	const std::string & s )
{
	const size_t begin_pos = s.find_first_not_of( g_spaces );

	if( std::string::npos == begin_pos )
		return std::string(); // строка s состоит только из пробелов.

	const size_t end_pos = s.find_last_not_of( g_spaces );

	return s.substr( begin_pos, end_pos - begin_pos + 1 );
}

} /* ananymous namespace */


endpoint_stage_chain_t
endpoint_stage_chain_t::create_from_string(
	const std::string & s )
{
	const size_t stages_desc_begin_pos = s.find( '[' );

	if( std::string::npos == stages_desc_begin_pos )
	{
		// »меем дело только с именем конечной точки.
		return endpoint_stage_chain_t( trimspaces( s ) );
	}

	endpoint_stage_chain_t
		res( trimspaces( s.substr( 0, stages_desc_begin_pos ) ) );

	const size_t stages_desc_end_pos = s.find( ']', stages_desc_begin_pos );

	if(
		// ≈сли ']' не найден,
		std::string::npos == stages_desc_end_pos ||
		// либо после этого еще есть какие-то не пробельные символы,
		std::string::npos !=
			s.find_first_not_of( g_spaces, stages_desc_end_pos + 1 ) )
	{
		// то формат не правильный.
		throw so_5::exception_t(
				"invalid stage description format: " + s,
				rc_invalid_stage_desc_format );
	}

	// ¬ырезаем то что находитс€ в [].
	const std::string & stages = trimspaces(
		s.substr(
			stages_desc_begin_pos + 1 ,
			stages_desc_end_pos - stages_desc_begin_pos - 1 ) );

	if( !stages.empty() )
	{
		stage_list_t stage_list;
		size_t pos = 0;
		while( true )
		{
			const size_t comma_pos = stages.find( ',', pos );

			if( std::string::npos != comma_pos )
			{
				stage_list.push_back(
					trimspaces( stages.substr( pos, comma_pos - pos ) ) );

				pos = comma_pos + 1;
			}
			else
			{
				stage_list.push_back(
					trimspaces( stages.substr( pos ) ) );
				break;
			}
		}

		res.set_stages( stage_list );
	}

	return res;
}

} /* namespace mbapi_4 */
