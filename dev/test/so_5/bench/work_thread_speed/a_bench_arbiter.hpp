/*
 * An agent which registers cooperation of benchmarks agents,
 * fixes moments of the start and the end of work.
 */

#if !defined( _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_ARBITER_HPP_ )
#define _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_ARBITER_HPP_

#include <vector>
#include <set>

#include <ace/Time_Value.h>

#include <so_5/rt/h/rt.hpp>

// A message to finish processing.
struct msg_bench_finish
	:
		public so_5::rt::message_t
{
	msg_bench_finish( unsigned int message_count )
		:
			m_message_count( message_count )
	{}

	virtual ~msg_bench_finish()
	{}

	unsigned int m_message_count;
};

// A message to start processing.
struct msg_bench_start
	: public so_5::rt::signal_t
{
};



// A benchmark arbiter class.
class a_bench_arbiter_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_bench_arbiter_t(
			so_5::rt::so_environment_t & env,
			const std::string & cfg_file )
			:
				base_type_t( env ),
				m_self_mbox( so_environment().create_local_mbox() ),
				m_cfg_file( cfg_file ),
				m_total_message_count( 0 ),
				m_recivers_total( 0 ),
				m_recivers_in_work( 0 )
		{}

		virtual ~a_bench_arbiter_t()
		{}

		virtual void
		so_define_agent();

		// Create benchmark agents.
		// Fix the moment of start.
		// Start benchmark.
		void
		so_evt_start();

		// A reaction to message of end of processing from some agent.
		void
		evt_on_bench_finish(
			const so_5::rt::event_data_t< msg_bench_finish > & msg );

	private:
		so_5::rt::mbox_ref_t	m_self_mbox;

		// A total count of receivers.
		unsigned int m_recivers_total;

		// A count of currently working receivers.
		unsigned int m_recivers_in_work;

		// A total count of messages processed.
		unsigned int m_total_message_count;

		// Name of config file.
		const std::string m_cfg_file;

		// Timestamp of start of benchmark.
		ACE_Time_Value m_start_time;

		std::vector< unsigned int > m_msg_bloks_size;

		// Names of senders.
		std::set< std::string >  m_senders;
		// Names of receivers.
		std::set< std::string >  m_receivers;
};

#endif
