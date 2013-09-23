/*
 * An agent which receives and handles messages.
 */

#if !defined( _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_RECEIVER_HPP_ )
#define _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_RECEIVER_HPP_

#include <so_5/rt/h/rt.hpp>

// Test message.
struct msg_some_message
	:
		public so_5::rt::message_t
{
	msg_some_message( unsigned int message_id )
		:
			m_message_id(message_id)
	{}

	virtual ~msg_some_message()
	{}

	unsigned int m_message_id;
};

// Message to finish work.
struct msg_finish_work
	: public so_5::rt::signal_t
{};

// A receiver for bunch of messages.
class a_bench_receiver_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_bench_receiver_t(
			so_5::rt::so_environment_t & env,
			// Receiver mbox.
			const so_5::rt::mbox_ref_t & target_mbox,
			// Arbiter mbox.
			const so_5::rt::mbox_ref_t & arbiter_mbox )
			:
				base_type_t( env ),
				m_target_mbox( target_mbox ),
				m_arbiter_mbox( arbiter_mbox ),
				m_processed_msg_count( 0 )
		{}

		virtual ~a_bench_receiver_t()
		{
		}

		virtual void
		so_define_agent();

		// Handle test message.
		void
		evt_bench(
			const so_5::rt::event_data_t< msg_some_message > & msg );

		// Handle end of work.
		void
		evt_finish(
			const so_5::rt::event_data_t< msg_finish_work > & msg );

	private:
		// Receiver mbox.
		so_5::rt::mbox_ref_t	m_target_mbox;

		// Arbiter mbox.
		so_5::rt::mbox_ref_t	m_arbiter_mbox;

		// Count of messages processed.
		unsigned int m_processed_msg_count;
};

#endif
