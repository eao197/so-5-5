#include <iostream>
#include <string>

#include <test/so_5/bench/time_value_msec_helper.hpp>

#include <microbenchmarks/so_5/demand_queue_1/demand_queue.hpp>

class pinger_t;
class ponger_t;

void
initiate_ping( work_thread_t & ponger_context, ponger_t * ponger );

void
initiate_pong( work_thread_t & pinger_context, pinger_t * pinger );

class pinger_t
{
	private :
		work_thread_t & m_self_context;
		work_thread_t & m_ponger_context;

		ponger_t * m_ponger;

		const size_t m_total_pong_count;
		size_t m_pong_count;

		ACE_Time_Value m_start_time;
		ACE_Time_Value m_finish_time;

	public :
		pinger_t(
			work_thread_t & self_context,
			work_thread_t & ponger_context,
			size_t total_pong_count )
			:	m_self_context( self_context )
			,	m_ponger_context( ponger_context )
			,	m_ponger( nullptr )
			,	m_total_pong_count( total_pong_count )
			,	m_pong_count( 0 )
			,	m_start_time( ACE_OS::gettimeofday() )
			,	m_finish_time( ACE_OS::gettimeofday() )
		{}

		void
		set_ponger( ponger_t * ponger )
		{
			m_ponger = ponger;
		}

		ACE_UINT64
		duration() const
		{
			return milliseconds( m_finish_time - m_start_time );
		}

		static void
		on_start( void * param )
		{
//			SetThreadAffinityMask( GetCurrentThread(), 0x1 );

			auto p = reinterpret_cast< pinger_t * >(param);

			p->m_start_time = ACE_OS::gettimeofday();

			initiate_ping( p->m_ponger_context, p->m_ponger );
		}

		static void
		on_pong( void * param )
		{
			auto p = reinterpret_cast< pinger_t * >(param);

			if( ++(p->m_pong_count) < p->m_total_pong_count )
				initiate_ping( p->m_ponger_context, p->m_ponger );
			else
				p->m_self_context.put_event( &on_finish, param );
		}

		static void
		on_finish( void * param )
		{
			auto p = reinterpret_cast< pinger_t * >(param);

			p->m_finish_time = ACE_OS::gettimeofday();
			p->m_self_context.shutdown();
			p->m_ponger_context.shutdown();
		}
};

class ponger_t
{
	private :
		work_thread_t & m_pinger_context;

		pinger_t * m_pinger;

	public :
		ponger_t(
			work_thread_t & pinger_context )
			:	m_pinger_context( pinger_context )
			,	m_pinger( nullptr )
		{}

		void
		set_pinger( pinger_t * pinger )
		{
			m_pinger = pinger;
		}

		static void
		on_start( void * param )
		{
//			SetThreadAffinityMask( GetCurrentThread(), 0x2 );
		}

		static void
		on_ping( void * param )
		{
			auto p = reinterpret_cast< ponger_t * >(param);

			initiate_pong( p->m_pinger_context, p->m_pinger );
		}
};

void
initiate_ping( work_thread_t & ponger_context, ponger_t * ponger )
{
	ponger_context.put_event( &ponger_t::on_ping, ponger );
}

void
initiate_pong( work_thread_t & pinger_context, pinger_t * pinger )
{
	pinger_context.put_event( &pinger_t::on_pong, pinger );
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
	work_thread_t pinger_context;
	work_thread_t ponger_context;

	const size_t iterations = iteration_count( argc, argv );
	std::cout << "iterations: " << iterations << std::endl;
	
	pinger_t pinger(
			pinger_context,
			ponger_context,
			iterations );
	ponger_t ponger( pinger_context );

	pinger.set_ponger( &ponger );
	ponger.set_pinger( &pinger );

	pinger_context.put_event( &pinger_t::on_start, &pinger );
	ponger_context.put_event( &ponger_t::on_start, &ponger );

	ponger_context.start();
	pinger_context.start();

	ponger_context.wait();
	pinger_context.wait();

	auto duration_sec = pinger.duration() / 1000.0;
	auto price = duration_sec / iterations;
	auto per_sec = 1 / price;

	std::cout.precision( 12 );
	std::cout << "Time: " << duration_sec << "s, per demand: "
			<< price << "s, per sec: " << per_sec
			<< std::endl;

	return 0;
}

