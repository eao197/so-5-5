/*
 * An agent which registers cooperation of benchmarks agents,
 * fixes moments of the start and the end of work.
 */

#include <exception>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include <ace/OS.h>

#include <cls_3/utils/h/parse_stream.hpp>

#include <so_5/disp/active_group/h/pub.hpp>

#include <test/so_5/bench/time_value_msec_helper.hpp>

#include "a_bench_sender.hpp"
#include "a_bench_receiver.hpp"
#include "a_bench_arbiter.hpp"
#include "tag_bench_cfg.hpp"


void
a_bench_arbiter_t::so_define_agent()
{
	so_subscribe( m_self_mbox )
		.event( &a_bench_arbiter_t::evt_on_bench_finish );
}

void
a_bench_arbiter_t::so_evt_start()
{
	try
	{
		tag_wt_bench_t tag( "work_thread_bench", true );
		cls_3::utils::root_tags_t tags;
		tags.push_back( &tag );

		cls_3::utils::parse_file(
			m_cfg_file,
			tags );

		if( tag.m_msg_blocks_size.is_defined() )
			m_msg_bloks_size.insert(
				m_msg_bloks_size.begin(),
				tag.m_msg_blocks_size.begin(),
				tag.m_msg_blocks_size.end() );
		else
			m_msg_bloks_size.push_back( 100 );

		if( 0 == tag.m_bench_pair.size() )
			throw std::runtime_error(
				"0 bench agents found in cfg file" );

		// Registering all sender-receiver pairs.
		for( int i = 0; i < tag.m_bench_pair.size(); ++i )
		{
			const tag_bench_pair_t & curr = tag.m_bench_pair.at( i );

			// Receiver mbox.
			so_5::rt::mbox_ref_t reciver_mbox =
				so_environment().create_local_mbox();

			// Registering receiver.
			{
				so_5::rt::agent_coop_unique_ptr_t coop =
					so_environment().create_coop(
						so_5::rt::nonempty_name_t(
							curr.m_receiver.query_value() ) );

				coop->add_agent(
					so_5::rt::agent_ref_t( new a_bench_receiver_t(
						so_environment(),
						reciver_mbox,
						m_self_mbox ) ),
					so_5::disp::active_group::create_disp_binder(
						"active_group",
						curr.m_receiver.m_active_group.query_value() ) );

				so_5::ret_code_t rc = so_environment().register_coop(
					std::move( coop ) );

				if( rc )
				{
					throw std::runtime_error(
						"unable to register " +
						curr.m_receiver.query_value() );
				}
			}
			++m_recivers_total;

			// Registering sender.
			{
				so_5::rt::agent_coop_unique_ptr_t coop =
					so_environment().create_coop(
						so_5::rt::nonempty_name_t(
							curr.m_sender.query_value() ) );

				coop->add_agent(
					so_5::rt::agent_ref_t( new a_bench_sender_t(
						so_environment(),
						reciver_mbox,
						m_self_mbox,
						curr.m_message_count.query_value(),
						m_msg_bloks_size ) ),
					so_5::disp::active_group::create_disp_binder(
						"active_group",
						curr.m_sender.m_active_group.query_value() ) );

				so_5::ret_code_t rc = so_environment().register_coop(
					std::move( coop ) );

				if( rc )
				{
					throw std::runtime_error(
						"unable to register " +
						curr.m_sender.query_value() );
				}
			}
		}
	}
	catch( std::exception & ex)
	{
		std::cout << ex.what() << std::endl;
		so_environment().stop();
		return;
	}

	m_recivers_in_work = m_recivers_total;
	// Fixing time and...
	m_start_time = ACE_OS::gettimeofday();

	// ...staring the benchmark.
	m_self_mbox->deliver_message< msg_bench_start >();
}

void
a_bench_arbiter_t::evt_on_bench_finish(
	const so_5::rt::event_data_t< msg_bench_finish > & msg )
{
	m_total_message_count += msg->m_message_count;
	--m_recivers_in_work;

	if( 0 == m_recivers_in_work )
	{
		double work_time =
			( milliseconds( ACE_OS::gettimeofday() ) -
			milliseconds( m_start_time ) ) / 1000.0;

		std::cout
			<< "messages: " << m_total_message_count
			<< ", total time: "<< std::fixed << work_time << " sec"
			<< ", throughput: " << std::fixed
			<< m_total_message_count / work_time << " msgs/sec"
			<< std::endl;

		// Shutting down SObjectizer.
		so_environment().stop();
	}
}

