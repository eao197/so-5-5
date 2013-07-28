/*
	јгент, который регистрирует кооперацию нагрузочных агентов,
	засекает врем€ и отмечает завершение работы.
*/

#if !defined( _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_ARBITER_HPP_ )
#define _SO_5__TEST__BENCH__WORK_THREAD_SPEED__A_BENCH_ARBITER_HPP_

#include <vector>
#include <set>

#include <ace/Time_Value.h>

#include <so_5/rt/h/rt.hpp>

// —ообщение о том что некоторый агент
// завершил обрабатывать сообщени€.
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

// —ообщение о том получател€м надо начать посылать сообщени€.
struct msg_bench_start
	:
		public so_5::rt::message_t
{
	virtual ~msg_bench_start()
	{}
};



// јгент который нагружаетс€ пачками сообщений.
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

		// ќпределение агента.
		virtual void
		so_define_agent();

		// ќбработка начала работы агента в системе.
		// —тарт теста. –егистрируетс€ тестова€ коопераци€
		// и засекаетс€ врем€.
		void
		so_evt_start();

		// ќбработка сообщени€ о завершении
		// обработки сообщений некоторым агентом.
		void
		evt_on_bench_finish(
			const so_5::rt::event_data_t< msg_bench_finish > & msg );

	private:
		// —обственный mbox агента.
		so_5::rt::mbox_ref_t	m_self_mbox;

		//  оличество агентов которые еще работают.
		unsigned int m_recivers_total;

		//  оличество агентов которые еще работают.
		unsigned int m_recivers_in_work;

		//  оличество обработанных сообщений.
		unsigned int m_total_message_count;

		// »м€ конфига теста.
		const std::string m_cfg_file;

		// «асечка начала работы.
		ACE_Time_Value m_start_time;

		std::vector< unsigned int > m_msg_bloks_size;

		std::set< std::string >  m_senders;
		std::set< std::string >  m_receivers;
};

#endif
