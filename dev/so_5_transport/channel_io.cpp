/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/channel_io.hpp>

namespace so_5_transport
{

//
// channel_io_t
//

channel_io_t::channel_io_t()
{
}

channel_io_t::channel_io_t(
	const scattered_block_stream_ref_t & incoming,
	const scattered_block_stream_ref_t & outgoing )
	:
		m_input( incoming ),
		m_output( outgoing )
{
}

channel_io_t::~channel_io_t()
{
}

input_trx_unique_ptr_t
channel_io_t::begin_input_trx() const
{
	input_trx_unique_ptr_t res;

	if( m_input.get() )
	{
		res.reset(
			new input_trx_t(
				channel_input_stream_unique_ptr_t(
					new channel_input_stream_t( m_input ) ) ) );
	}

	return res;
}

output_trx_unique_ptr_t
channel_io_t::begin_output_trx(
	const ACE_Time_Value & free_blocks_awaiting_timeout ) const
{
	output_trx_unique_ptr_t res;

	if( m_output.get() )
	{
		res.reset(
			new output_trx_t(
				channel_output_stream_unique_ptr_t(
					new channel_output_stream_t(
						m_output,
						free_blocks_awaiting_timeout ) ) ) );
	}

	return res;
}

// void
// channel_io_t::release_streams()
// {
	// m_input = scattered_block_stream_ref_t();
	// m_output = scattered_block_stream_ref_t();
// }


// void
// channel_io_t::set_streams(
	// channel_io_t & channel_io,
	// const scattered_block_stream_ref_t & in,
	// const scattered_block_stream_ref_t & out )
// {
	// channel_io.set_incoming_stream( in );
	// channel_io.set_outgoing_stream( out );
// }

// void
// channel_io_t::set_incoming_stream(
	// channel_io_t & channel_io,
	// const scattered_block_stream_ref_t & in )
// {
	// channel_io.set_incoming_stream( in );
// }

// void
// channel_io_t::set_outgoing_stream(
	// channel_io_t & channel_io,
	// const scattered_block_stream_ref_t & out )
// {
	// channel_io.set_outgoing_stream( out );
// }

// void
// channel_io_t::set_incoming_stream(
	// const scattered_block_stream_ref_t & in )
// {
	// m_input = in;
// }

// void
// channel_io_t::set_outgoing_stream(
	// const scattered_block_stream_ref_t & out )
// {
	// m_output = out;
// }

} /* namespace so_5_transport */

