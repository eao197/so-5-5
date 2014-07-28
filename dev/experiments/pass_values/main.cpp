#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <atomic>

#include <test/so_5/bench/benchmark_helpers.hpp>

const std::size_t cycles_count = 1000000;
const std::size_t vector_size = 1;

struct demand_t
{
	void * m_ptr1 = nullptr;
	void * m_ptr2 = nullptr;
	void * m_ptr3 = nullptr;
	void * m_ptr4 = nullptr;
	void * m_ptr5 = nullptr;
	void * m_ptr6 = nullptr;
};

struct demand_vector_t
{
	std::atomic_uint m_vector_size;

	std::vector< demand_t * > m_demands;
};

void
thread_body(
	demand_vector_t * to_fill,
	demand_vector_t * to_clear )
{
	demand_t tmp;

	for( std::size_t c = 0; c != cycles_count; ++c )
	{
		while( 0 != to_fill->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();

		for( auto & p : to_fill->m_demands )
			p = &tmp;

		to_fill->m_vector_size.store(
				to_fill->m_demands.size(),
				std::memory_order_release );

		while( 0 == to_clear->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();
		for( auto & p : to_clear->m_demands )
		{
			p = nullptr;
		}

		to_clear->m_vector_size.store( 0, std::memory_order_release );
	}
}

void
do_test( unsigned int thread_count )
{
	std::vector< demand_vector_t > demand_vectors( thread_count,
			demand_vector_t() );

	for( auto & v : demand_vectors )
	{
		v.m_vector_size.store( 0, std::memory_order_release );
		v.m_demands.resize( vector_size, nullptr );
	}

	benchmarker_t benchmarker;
	benchmarker.start();

	std::vector< std::thread > threads;
	threads.reserve( thread_count );
	for( std::size_t i = 0; i != thread_count; ++i )
		threads.emplace_back( thread_body,
				&(demand_vectors[ i ]),
				&(demand_vectors[ (i+1) % thread_count ]) );

	for( auto & t : threads )
		t.join();

	benchmarker.finish_and_show_stats( 
			thread_count * cycles_count, "cycles" );
}

int main( int argc, char ** argv )
{
	unsigned int thread_count = std::thread::hardware_concurrency();

	if( 1 < argc )
		thread_count = std::atoi( argv[ 1 ] );

	std::cout << "threads: " << thread_count << "\n..." << std::endl;

	do_test( thread_count );
}


