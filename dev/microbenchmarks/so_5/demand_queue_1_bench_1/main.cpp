#include <iostream>
#include <string>

#include <test/so_5/bench/time_value_msec_helper.hpp>

//#include <microbenchmarks/so_5/demand_queue_1/demand_queue.hpp>
#include <microbenchmarks/so_5/demand_queue_1_naive_spinlock/demand_queue.hpp>

class agent_imitator_t
{
	private :
		work_thread_t & m_context;

		const size_t m_iteration_count;
		size_t m_current_iteration;

		ACE_Time_Value m_start_time;
		ACE_Time_Value m_finish_time;

	public :
		agent_imitator_t(
			work_thread_t & context,
			size_t iteration_count )
			:	m_context( context )
			,	m_iteration_count( iteration_count )
			,	m_current_iteration( 0 )
			,	m_start_time( ACE_OS::gettimeofday() )
			,	m_finish_time( ACE_OS::gettimeofday() )
		{}

		ACE_UINT64
		duration() const
		{
			return milliseconds( m_finish_time - m_start_time );
		}

		static void
		on_start( void * param )
		{
			auto p = reinterpret_cast< agent_imitator_t * >(param);
			p->m_start_time = ACE_OS::gettimeofday();

			p->m_context.put_event( &on_demand, param );
		}

		static void
		on_demand( void * param )
		{
			auto p = reinterpret_cast< agent_imitator_t * >(param);

			if( ++(p->m_current_iteration) < p->m_iteration_count )
				p->m_context.put_event( &on_demand, param );
			else
				p->m_context.put_event( &on_finish, param );
		}

		static void
		on_finish( void * param )
		{
			auto p = reinterpret_cast< agent_imitator_t * >(param);

			p->m_finish_time = ACE_OS::gettimeofday();
			p->m_context.shutdown();
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

	agent_imitator_t agent( context, iterations );

	context.put_event( &agent_imitator_t::on_start, &agent );
	context.start();
	context.wait();

	auto duration_sec = agent.duration() / 1000.0;
	auto price = duration_sec / iterations;
	auto per_sec = 1 / price;

	std::cout.precision( 12 );
	std::cout << "Time: " << duration_sec << "s, per demand: "
			<< price << "s, per sec: " << per_sec
			<< std::endl;

	return 0;
}

