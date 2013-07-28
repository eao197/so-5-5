/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/messages.hpp>

namespace so_5_transport
{

//
// msg_create_server_socket_result
//

msg_create_server_socket_result::msg_create_server_socket_result()
	:
		m_succeed( false )
{
}

msg_create_server_socket_result::msg_create_server_socket_result(
	bool succeed )
	:
		m_succeed( succeed )
{
}

//
// msg_channel_created
//

msg_channel_created::msg_channel_created()
{
}

msg_channel_created::msg_channel_created(
	const channel_id_t & id,
	const channel_io_t & io,
	const channel_controller_ref_t & controller )
	:
		m_channel_id( id ),
		m_io( io ),
		m_controller( controller )
{
}

msg_channel_created::~msg_channel_created()
{
}

//
// msg_channel_failed
//

msg_channel_failed::msg_channel_failed()
{
}

msg_channel_failed::msg_channel_failed(
	const std::string & reason )
	:
		m_reason( reason )
{
}

msg_channel_failed::~msg_channel_failed()
{
}

//
// msg_channel_lost
//

msg_channel_lost::msg_channel_lost()
{
}

msg_channel_lost::msg_channel_lost(
	const channel_id_t & id,
	const std::string & reason,
	const channel_controller_ref_t & controller )
	:
		m_channel_id( id ),
		m_reason( reason ),
		m_controller( controller )
{
}

msg_channel_lost::~msg_channel_lost()
{
}

//
// msg_incoming_package
//

msg_incoming_package::msg_incoming_package()
	:
		m_channel_id( 0 ),
		m_input_transaction_took_place( false )
{
}

msg_incoming_package::msg_incoming_package(
	channel_id_t channel_id,
	const channel_io_t & io )
	:
		m_channel_id( channel_id ),
		m_io( io ),
		m_input_transaction_took_place( false )
{
}

msg_incoming_package::~msg_incoming_package()
{
	// Если транзакция чтения не начиналась,
	// то увеличиваем версию потока.
	if( !m_input_transaction_took_place )
	{
		scattered_block_stream_ref_t sbs =
			channel_io_t::access_input( m_io );

		sbs->touch_stream();
	}
}

input_trx_unique_ptr_t
msg_incoming_package::begin_input_trx() const
{
	if( !m_input_transaction_took_place )
	{
		m_input_transaction_took_place = true;
		return m_io.begin_input_trx();
	}

	return input_trx_unique_ptr_t();
}

output_trx_unique_ptr_t
msg_incoming_package::begin_output_trx(
	const ACE_Time_Value &
		free_blocks_awaiting_timeout ) const
{
	return m_io.begin_output_trx( free_blocks_awaiting_timeout );
}

} /* namespace so_5_transport */
