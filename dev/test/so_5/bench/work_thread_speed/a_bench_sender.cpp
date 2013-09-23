/* 
 * An agent which sends messages.
 */
#include <iostream>

#include "a_bench_sender.hpp"
#include "a_bench_receiver.hpp"

void
a_bench_sender_t::so_define_agent()
{
	so_subscribe( m_arbiter_mbox )
		.event( &a_bench_sender_t::evt_generate_pack );

	so_subscribe( m_self_mbox )
		.event( &a_bench_sender_t::evt_generate_pack );
}

void
a_bench_sender_t::evt_generate_pack(
	const so_5::rt::event_data_t< msg_bench_start > & msg )
{
	const unsigned int curr_pack_size = 
		m_packs_size[ m_iteration_count % m_packs_size.size() ];
	++m_iteration_count;

	for( int i = 0; 
		i < curr_pack_size && m_sent_message_count < m_message_count;
		++i, ++m_sent_message_count )
	{
		m_target_mbox->deliver_message(
			std::unique_ptr< msg_some_message >( 
				new msg_some_message( m_sent_message_count ) ) );
	}

	if( m_sent_message_count < m_message_count )
	{
		// Not all messages sent. We should reinitiate youself.
		m_self_mbox->deliver_signal< msg_bench_start >();
	}
	else
	{
		// All messages send. We should finish work.
		m_target_mbox->deliver_signal< msg_finish_work >();
	}
}
