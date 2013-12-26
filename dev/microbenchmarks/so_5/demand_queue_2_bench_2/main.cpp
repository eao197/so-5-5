#include <iostream>
#include <string>
#include <ctime>

#include <microbenchmarks/so_5/demand_queue_2/demand_queue.hpp>

class a_pinger_t;
class a_ponger_t;

void
initiate_ping( a_ponger_t & ponger );

void
initiate_pong( a_pinger_t & pinger );

class a_pinger_t
	:	public agent_t
	{
	private :
		work_thread_t & m_context;

		a_ponger_t & m_ponger;

		const size_t m_iteration_count;
		size_t m_current_iteration;

		std::clock_t m_start_time;
		std::clock_t m_finish_time;

		class start_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_pinger_t * >(agent)->on_start( msg );
					}
			}
		m_start_event_caller;

		class pong_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_pinger_t * >(agent)->on_pong( msg );
					}
			}
		m_pong_event_caller;

		class finish_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_pinger_t * >(agent)->on_finish( msg );
					}
			}
		m_finish_event_caller;

	public :
		a_pinger_t(
			work_thread_t & context,
			a_ponger_t & ponger,
			size_t iteration_count )
			:	agent_t(
					std::unique_ptr< agent_demand_storage_t >(
							new simple_demand_storage_t() ),
					context.dispatcher_notificator() )
			,	m_context( context )
			,	m_ponger( ponger )
			,	m_iteration_count( iteration_count )
			,	m_current_iteration( 0 )
			{}

		double
		duration() const
			{
				return double(m_finish_time - m_start_time) / CLOCKS_PER_SEC;
			}

		void
		on_start( message_t * )
			{
				m_start_time = std::clock();

				send_msg_ping();
			}

		void
		on_pong( message_t * )
			{
				if( ++m_current_iteration < m_iteration_count )
					send_msg_ping();
				else
					send_finish();
			}

		void
		on_finish( message_t * )
			{
				m_finish_time = std::clock();
				m_context.shutdown();
			}

		void
		send_start()
			{
				send_message( nullptr, m_start_event_caller );
			}

		void
		send_pong()
			{
				send_message( nullptr, m_pong_event_caller );
			}

	private :
		void
		send_msg_ping()
			{
				initiate_ping( m_ponger );
			}

		void
		send_finish()
			{
				send_message( nullptr, m_finish_event_caller );
			}
	};

class a_ponger_t
	:	public agent_t
	{
	private :
		a_pinger_t * m_pinger;

		class start_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_ponger_t * >(agent)->on_start( msg );
					}
			}
		m_start_event_caller;

		class ping_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_ponger_t * >(agent)->on_ping( msg );
					}
			}
		m_ping_event_caller;

	public :
		a_ponger_t(
			work_thread_t & context )
			:	agent_t(
					std::unique_ptr< agent_demand_storage_t >(
							new simple_demand_storage_t() ),
					context.dispatcher_notificator() )
			{}

		void
		set_pinger( a_pinger_t & pinger )
			{
				m_pinger = &pinger;
			}

		void
		on_start( message_t * )
			{
			}

		void
		on_ping( message_t * )
			{
				initiate_pong( *m_pinger );
			}

		void
		send_start()
			{
				send_message( nullptr, m_start_event_caller );
			}

		void
		send_ping()
			{
				send_message( nullptr, m_ping_event_caller );
			}
	};

void
initiate_ping( a_ponger_t & ponger )
	{
		ponger.send_ping();
	}

void
initiate_pong( a_pinger_t & pinger )
	{
		pinger.send_pong();
	}

size_t
iteration_count( int argc, char ** argv )
	{
		if( argc > 1 )
			{
				return std::stoul( argv[ 1 ] );
			}

		return 1000000;
	}

int
main(int argc, char ** argv)
	{
		work_thread_t context_pinger;
		work_thread_t context_ponger;

		const size_t iterations = iteration_count( argc, argv );
		std::cout << "iterations: " << iterations << std::endl;

		a_ponger_t ponger( context_ponger );
		a_pinger_t pinger( context_pinger, ponger, iterations );
		ponger.set_pinger( pinger );

		pinger.send_start();
		ponger.send_start();

		context_ponger.start();

		context_pinger.start();
		context_pinger.wait();

		context_ponger.shutdown();
		context_ponger.wait();

		auto duration_sec = pinger.duration();
		auto price = duration_sec / iterations;
		auto per_sec = 1 / price;

		std::cout.precision( 12 );
		std::cout << "Time: " << duration_sec << "s, per demand: "
				<< price << "s, per sec: " << per_sec
				<< std::endl;

		return 0;
	}

