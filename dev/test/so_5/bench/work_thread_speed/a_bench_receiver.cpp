/*
 * An agent which receives and handles messages.
 */

#include "a_bench_receiver.hpp"
#include "a_bench_arbiter.hpp"

void
a_bench_receiver_t::so_define_agent()
{
	so_subscribe( m_target_mbox )
		.event( &a_bench_receiver_t::evt_bench );

	so_subscribe( m_target_mbox )
		.event( &a_bench_receiver_t::evt_finish );
}

void
a_bench_receiver_t::evt_bench(
	const so_5::rt::event_data_t< msg_some_message > & msg )
{
	++m_processed_msg_count;
}

void
a_bench_receiver_t::evt_finish(
	const so_5::rt::event_data_t< msg_finish_work > & msg )
{
	m_arbiter_mbox->deliver_message(
		std::unique_ptr< msg_bench_finish >(
			new msg_bench_finish( m_processed_msg_count ) ) );
}
