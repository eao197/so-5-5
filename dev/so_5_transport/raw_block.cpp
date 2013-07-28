/*
	SObjectizer 5 Transport.
*/

#include <algorithm>

#include <so_5_transport/h/raw_block.hpp>

namespace so_5_transport
{

//
// raw_block_t
//

raw_block_t::raw_block_t(
	size_t capacity )
	:
		m_write_pos( 0 ),
		m_read_pos( 0 )
{
	m_block.reserve( capacity );
	m_block.push_back( 0 );
	m_buff_ptr = &m_block.front();
}

raw_block_t::~raw_block_t()
{
}

} /* namespace so_5_transport */

