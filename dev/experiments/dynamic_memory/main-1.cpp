#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>

#include <test/so_5/bench/benchmark_helpers.hpp>

const std::size_t allocation_count = 1000000;

struct demand_t
{
	void * m_ptr1 = nullptr;
	void * m_ptr2 = nullptr;
	void * m_ptr3 = nullptr;
	void * m_ptr4 = nullptr;
	void * m_ptr5 = nullptr;
	void * m_ptr6 = nullptr;
};

void
thread_body()
{
	for( std::size_t i = 0; i != allocation_count; ++i )
	{
		auto d = new demand_t;
		if( i < allocation_count )
			delete d;
	}
}

void
do_test( unsigned int thread_count )
{
	benchmarker_t benchmarker;
	benchmarker.start();

	std::vector< std::thread > threads;
	threads.reserve( thread_count );
	for( std::size_t i = 0; i != thread_count; ++i )
		threads.emplace_back( thread_body );

	for( auto & t : threads )
		t.join();

	benchmarker.finish_and_show_stats( 
			thread_count * allocation_count, "allocs" );
}

int main( int argc, char ** argv )
{
	unsigned int thread_count = std::thread::hardware_concurrency();

	if( 1 < argc )
		thread_count = std::atoi( argv[ 1 ] );

	std::cout << "threads: " << thread_count << "\n..." << std::endl;

	do_test( thread_count );
}


