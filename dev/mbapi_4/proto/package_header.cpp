/*
	MBAPI 4.
*/

#include <cstdio>
#include <iostream>

#include <cpp_util_2/h/lexcast.hpp>
#include <cpp_util_2/lexcasts/h/util.hpp>

#include <mbapi_4/proto/h/package_header.hpp>

namespace mbapi_4
{

namespace proto
{

//
// package_header_t
//

package_header_t::package_header_t(
	const magic_number_t & value )
	:
		m_magic_number( value.m_value ),
		m_size( 0 )
{
	calc_check_field();
}

package_header_t::package_header_t(
	const magic_number_t & value,
	uint32_t package_size )
	:
		m_magic_number( value.m_value ),
		m_size( 0 )
{
	set_size( package_size );
}

bool
package_header_t::is_valid() const
{
	return ( m_check_field == ( m_size ^ m_magic_number ) );
}

package_header_t::uint32_t
package_header_t::size() const
{
	return m_size;
}

void
package_header_t::set_size( uint32_t package_size )
{
	m_size = package_size;
	calc_check_field();
}

void
package_header_t::read( oess_2::io::ibinstream_t & from )
{
	from >> m_size >> m_check_field;
}

void
package_header_t::write( oess_2::io::obinstream_t & to ) const
{
	to.write( &m_size, 1 );
	to.write( &m_check_field, 1 );
}

std::string
package_header_t::debug_string_representation() const
{
	return "{header {magic_number " +
			cpp_util_2::slexcast(
					m_magic_number,
					cpp_util_2::lexcasts::hex_0x() ) +
			"} {size " +
			cpp_util_2::slexcast( m_size,
					cpp_util_2::lexcasts::hex_0x() ) +
			"} {check_field " +
			cpp_util_2::slexcast(
					m_check_field,
					cpp_util_2::lexcasts::hex_0x() ) +
			"} {expected_check_field " +
			cpp_util_2::slexcast(
					m_size ^ m_magic_number,
					cpp_util_2::lexcasts::hex_0x() ) +
			"} }";
}

void
package_header_t::calc_check_field()
{
	m_check_field = m_size ^ m_magic_number;
}

} /* namespace proto */

} /* namespace mbapi_4 */
