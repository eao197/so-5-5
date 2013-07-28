/*
	MBAPI 4.
*/
#include <oess_2/stdsn/h/inout_templ.hpp>

#include <mbapi_4/proto/h/handshake_info.hpp>

namespace mbapi_4
{

namespace proto
{

namespace handshake
{

//
// version_t
//

version_t::version_t()
	:
		m_generation( 0 ),
		m_branch( 0 ),
		m_release( 0 ),
		m_publish( 0 )
{
}

version_t::version_t(
	oess_2::uchar_t generation,
	oess_2::uchar_t branch,
	oess_2::uchar_t release,
	oess_2::uchar_t publish )
	:
		m_generation( generation ),
		m_branch( branch ),
		m_release( release ),
		m_publish( publish )
{
}

version_t::~version_t()
{
}

version_t
version_t::current()
{
	//FIX ME: обновлять при изменении версии!
	return version_t( 4, 0, 0, 0 );
}

//
// compression_t
//

const std::string
compression_t::zlib()
{
	return "zlib";
}

compression_t::compression_t()
{
}

compression_t::compression_t(
	const std::string & preferred,
	const std::set< std::string > & supported )
:	m_preferred( preferred )
,	m_supported( supported )
{
	if( preferred.length() )
		m_supported.insert( preferred );
}

compression_t::~compression_t()
{
}

compression_t::operator bool() const
{
	return ( 0 != m_preferred.length() &&
		is_supported( m_preferred ) );
}

bool
compression_t::is_supported( const std::string & name ) const
{
	return ( m_supported.find( name ) != m_supported.end() );
}

} /* namespace handshake */

//
// handshake_info_t
//

handshake_info_t::handshake_info_t()
	:
		req_info_t( req_id::REQ_HANDSHAKE ),
		m_version( handshake::version_t::current() )
{
}

handshake_info_t::handshake_info_t(
	const handshake::version_t & version )
	:
		req_info_t( req_id::REQ_HANDSHAKE ),
		m_version( version )
{
}

handshake_info_t::~handshake_info_t()
{
}

const handshake::version_t
handshake_info_t::version() const
{
	return m_version;
}

const handshake::compression_t
handshake_info_t::compression() const
{
	return m_compression;
}

void
handshake_info_t::set_compression(
	const handshake::compression_t & c )
{
	m_compression = c;
}

const std::string &
handshake_info_t::node_uid() const
{
	return m_node_uid;
}

void
handshake_info_t::set_node_uid( const std::string & uid )
{
	m_node_uid = uid;
}


//
// handshake_resp_info_t
//

handshake_resp_info_t::handshake_resp_info_t()
	:
		resp_info_t( req_id::RESP_HANDSHAKE, 0 ),
		m_version( handshake::version_t::current() )
{
}

handshake_resp_info_t::handshake_resp_info_t(
	const handshake::version_t & version )
	:
		resp_info_t( req_id::RESP_HANDSHAKE, 0 ),
		m_version( version )
{
}

handshake_resp_info_t::~handshake_resp_info_t()
{
}

const handshake::version_t
handshake_resp_info_t::version() const
{
	return m_version;
}

bool
handshake_resp_info_t::is_compression_defined() const
{
	return ( 0 != m_compression_algorithm.length() );
}

const std::string &
handshake_resp_info_t::compression_algorithm() const
{
	return m_compression_algorithm;
}

void
handshake_resp_info_t::set_compression_algorithm(
	const std::string & name )
{
	m_compression_algorithm = name;
}

const std::string &
handshake_resp_info_t::node_uid() const
{
	return m_node_uid;
}

void
handshake_resp_info_t::set_node_uid( const std::string & uid )
{
	m_node_uid = uid;
}


} /* namespace proto */

} /* namespace mbapi_4 */
