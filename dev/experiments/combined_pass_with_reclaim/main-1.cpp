#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class benchmarker_t
	{
	public :
		inline void
		start()
			{
				m_start = std::chrono::high_resolution_clock::now();
			}

		inline void
		finish_and_show_stats(
			unsigned long long events,
			const std::string & title )
			{
				auto finish_time = std::chrono::high_resolution_clock::now();
				const double duration =
						std::chrono::duration_cast< std::chrono::milliseconds >(
								finish_time - m_start ).count() / 1000.0;
				const double price = duration / events;
				const double throughtput = 1 / price;

				std::cout.precision( 10 );
				std::cout << title << ": " << events
						<< ", total_time: " << duration << "s"
						<< "\n""price: " << price << "s"
						<< "\n""throughtput: " << throughtput << " " << title << "/s"
						<< std::endl;
			}

	private :
		std::chrono::high_resolution_clock::time_point m_start;
	};

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
using demand_vector_shptr_t = std::shared_ptr< demand_vector_t >;

using alloc_func_t = std::function< demand_t *() >;
using dealloc_func_t = std::function< void( demand_t * ) >;

void
thread_body(
	demand_vector_t * own_fill,
	demand_vector_t * own_reclaim,
	demand_vector_t * alien_clear,
	demand_vector_t * alien_reclaim,
	alloc_func_t alloc_func,
	dealloc_func_t dealloc_func )
{
	for( std::size_t c = 0; c != cycles_count; ++c )
	{
		while( 0 != own_fill->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();

		for( auto & p : own_fill->m_demands )
		{
			p = alloc_func();
		}

		own_fill->m_vector_size.store(
				own_fill->m_demands.size(),
				std::memory_order_release );

		while( 0 == alien_clear->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();
		while( 0 != alien_reclaim->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();
		for( std::size_t i = 0; i != alien_clear->m_demands.size(); ++i )
		{
			alien_reclaim->m_demands[ i ] = alien_clear->m_demands[ i ];
			alien_clear->m_demands[ i ] = nullptr;
		}

		alien_reclaim->m_vector_size.store(
				alien_reclaim->m_demands.size(),
				std::memory_order_release );
		alien_clear->m_vector_size.store( 0, std::memory_order_release );

		while( 0 == own_reclaim->m_vector_size.load( std::memory_order_acquire ) )
			std::this_thread::yield();
		for( auto & p : own_reclaim->m_demands )
		{
			dealloc_func( p );
			p = nullptr;
		}
		own_reclaim->m_vector_size.store( 0, std::memory_order_release );
	}
}

void
do_test(
	unsigned int thread_count,
	unsigned int shift,
	alloc_func_t alloc_func,
	dealloc_func_t dealloc_func,
	const char * operation_name )
{
	std::vector< demand_vector_shptr_t > demand_vectors( thread_count,
			demand_vector_shptr_t() );
	std::vector< demand_vector_shptr_t > reclaim_vectors( thread_count,
			demand_vector_shptr_t() );

	for( auto & v : demand_vectors )
	{
		v = demand_vector_shptr_t( new demand_vector_t );
		v->m_vector_size.store( 0, std::memory_order_release );
		v->m_demands.resize( vector_size, nullptr );
	}

	for( auto & v : reclaim_vectors )
	{
		v = demand_vector_shptr_t( new demand_vector_t );
		v->m_vector_size.store( 0, std::memory_order_release );
		v->m_demands.resize( vector_size, nullptr );
	}


	benchmarker_t benchmarker;
	benchmarker.start();

	std::vector< std::thread > threads;
	threads.reserve( thread_count );
	for( std::size_t i = 0; i != thread_count; ++i )
		threads.emplace_back( thread_body,
				demand_vectors[ i ].get(),
				reclaim_vectors[ i ].get(),
				demand_vectors[ (i+shift) % thread_count ].get(),
				reclaim_vectors[ (i+shift) % thread_count ].get(),
				alloc_func,
				dealloc_func );

	for( auto & t : threads )
		t.join();

	benchmarker.finish_and_show_stats( 
			thread_count * cycles_count, operation_name );
}

demand_t * real_alloc() { return new demand_t; }
void real_dealloc( demand_t * p ) { delete p; }

demand_t * pseudo_alloc()
{
	static demand_t demand;
	return &demand;
}

void pseudo_dealloc( demand_t * p )
{}

int main( int argc, char ** argv )
{
	unsigned int thread_count = std::thread::hardware_concurrency();

	if( 1 < argc )
		thread_count = std::atoi( argv[ 1 ] );

	std::cout << "threads: " << thread_count << "\n..." << std::endl;

	std::cout << "*** shift=1 ***" << std::endl;
	do_test( thread_count, 1, real_alloc, real_dealloc, "allocs" );
	do_test( thread_count, 1, pseudo_alloc, pseudo_dealloc, "assigns" );

#if 0
	std::cout << "\n" "*** shift=0 ***" << std::endl;
	do_test( thread_count, 0, real_alloc, real_dealloc, "allocs" );
	do_test( thread_count, 0, pseudo_alloc, pseudo_dealloc, "assigns" );
#endif
}

