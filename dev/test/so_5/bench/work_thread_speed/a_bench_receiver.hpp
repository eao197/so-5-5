/*
	Агент, который принимает и обрабатывает сообщения.
*/

#if !defined( _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_RECEIVER_HPP_ )
#define _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_RECEIVER_HPP_

#include <so_5/rt/h/rt.hpp>

// Тестовое сообщение.
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

// Сообщение о завершении работы.
struct msg_finish_work
	:
		public so_5::rt::message_t
{
	virtual ~msg_finish_work()
	{}
};

// Агент который нагружается пачками сообщений.
class a_bench_receiver_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_bench_receiver_t(
			so_5::rt::so_environment_t & env,
			// mbox откуда принимать сообщения.
			const so_5::rt::mbox_ref_t & target_mbox,
			// mbox арбитра.
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

		// Обработка сообщений.
		void
		evt_bench(
			const so_5::rt::event_data_t< msg_some_message > & msg );

		// Завершение работы агента.
		// Обработка последнего сообщения.
		void
		evt_finish(
			const so_5::rt::event_data_t< msg_finish_work > & msg );

	private:
		// mbox, откуда принимать сообщения.
		so_5::rt::mbox_ref_t	m_target_mbox;

		// mbox арбитра.
		so_5::rt::mbox_ref_t	m_arbiter_mbox;

		// Количество обработанных тестовых сообщений.
		unsigned int m_processed_msg_count;
};

#endif
