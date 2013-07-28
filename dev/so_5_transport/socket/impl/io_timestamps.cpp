/*
	SObjectizer 5 Transport.
*/

#include <ace/OS.h>

#include <so_5_transport/socket/impl/h/io_timestamps.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// io_timestamps_t
//

//
// io_timestamps_t
//

io_timestamps_t::io_timestamps_t()
	:
		m_last_input( ACE_OS::gettimeofday() ),
		m_last_output( ACE_OS::gettimeofday() )
{
}

void
io_timestamps_t::mark_last_input()
{
	m_last_input = ACE_OS::gettimeofday();
}

void
io_timestamps_t::mark_last_output()
{
	m_last_output = ACE_OS::gettimeofday();
}

ACE_Time_Value
io_timestamps_t::since_last_input() const
{
	return ACE_OS::gettimeofday() - m_last_input;
}

ACE_Time_Value
io_timestamps_t::since_last_output() const
{
	return ACE_OS::gettimeofday() - m_last_output;
}

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */
