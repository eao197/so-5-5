/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/channel_params.hpp>

namespace so_5_transport
{

//
// channel_params_t
//

channel_params_t::channel_params_t()
	:
		m_time_checking_period( 1 ),
		m_max_input_block_timeout( 30 ),
		m_max_output_block_timeout( 30 )
{
}

channel_params_t::~channel_params_t()
{
}

const scattered_block_stream_params_t &
channel_params_t::channel_input_stream_params() const
{
	return m_channel_input_stream_params;
}

channel_params_t &
channel_params_t::set_channel_input_stream_params(
	const scattered_block_stream_params_t & v )
{
	m_channel_input_stream_params = v;
	return *this;
}

const scattered_block_stream_params_t &
channel_params_t::channel_output_stream_params() const
{
	return m_channel_output_stream_params;
}

channel_params_t &
channel_params_t::set_channel_output_stream_params(
	const scattered_block_stream_params_t & v )
{
	m_channel_output_stream_params = v;
	return *this;
}

unsigned int
channel_params_t::time_checking_period() const
{
	return m_time_checking_period;
}

channel_params_t &
channel_params_t::set_time_checking_period(
unsigned int v )
{
	m_time_checking_period = v;
	return *this;
}

unsigned int
channel_params_t::max_input_block_timeout() const
{
	return m_max_input_block_timeout;
}

channel_params_t &
channel_params_t::set_max_input_block_timeout(
unsigned int v )
{
	m_max_input_block_timeout = v;
	return *this;
}

unsigned int
channel_params_t::max_output_block_timeout() const
{
	return m_max_output_block_timeout;
}

channel_params_t &
channel_params_t::set_max_output_block_timeout(
unsigned int v )
{
	m_max_output_block_timeout = v;
	return *this;
}

} /* namespace so_5_transport */

