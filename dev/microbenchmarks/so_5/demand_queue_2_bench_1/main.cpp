#include <iostream>
#include <string>
#include <ctime>

#include <microbenchmarks/so_5/demand_queue_2/demand_queue.hpp>

class msg_demand_t :	public message_t {};

class a_benchmarker_t
	:	public agent_t
{
	private :
		work_thread_t & m_context;

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
						dynamic_cast< a_benchmarker_t * >(agent)->on_start( msg );
					}
			}
		m_start_event_caller;

		class demand_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_benchmarker_t * >(agent)->on_demand(
								dynamic_cast< msg_demand_t * >(msg) );
					}
			}
		m_demand_event_caller;

		class finish_event_caller_t
			:	public event_caller_t
			{
			public :
				virtual void
				call( agent_t * agent, message_t * msg )
					{
						dynamic_cast< a_benchmarker_t * >(agent)->on_finish( msg );
					}
			}
		m_finish_event_caller;

	public :
		a_benchmarker_t(
			work_thread_t & context,
			size_t iteration_count )
			:	agent_t(
					std::unique_ptr< agent_demand_storage_t >(
							new simple_demand_storage_t() ),
					context.dispatcher_notificator() )
			,	m_context( context )
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

				send_msg_demand();
			}

		void
		on_demand( msg_demand_t * msg )
			{
				if( ++m_current_iteration < m_iteration_count )
					send_msg_demand();
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

	private :
		void
		send_msg_demand()
			{
				send_message( nullptr, m_demand_event_caller );
			}

		void
		send_finish()
			{
				send_message( nullptr, m_finish_event_caller );
			}
	};

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
		work_thread_t context;

		const size_t iterations = iteration_count( argc, argv );
		std::cout << "iterations: " << iterations << std::endl;

		a_benchmarker_t agent( context, iterations );

		agent.send_start();

		context.start();
		context.wait();

		auto duration_sec = agent.duration();
		auto price = duration_sec / iterations;
		auto per_sec = 1 / price;

		std::cout.precision( 12 );
		std::cout << "Time: " << duration_sec << "s, per demand: "
				<< price << "s, per sec: " << per_sec
				<< std::endl;

		return 0;
	}

