#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <numeric>
#include <mutex>
#include <condition_variable>
#include <deque>

#include <so_5/h/spinlocks.hpp>

#include <test/so_5/bench/benchmark_helpers.hpp>

template<typename T>
class mpmc_bounded_queue
{
public:
	mpmc_bounded_queue(size_t buffer_size)
		: buffer_(new cell_t [buffer_size])
		, buffer_mask_(buffer_size - 1)
	{
		assert((buffer_size >= 2) &&
			((buffer_size & (buffer_size - 1)) == 0));

		for (size_t i = 0; i != buffer_size; i += 1)
			buffer_[i].sequence_.store(i, std::memory_order_relaxed);

		enqueue_pos_.store(0, std::memory_order_relaxed);
		dequeue_pos_.store(0, std::memory_order_relaxed);
	}

	~mpmc_bounded_queue()
	{
		delete [] buffer_;
	}

	bool enqueue(T const& data)
	{
		cell_t* cell;
		size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
		for (;;)
		{
			cell = &buffer_[pos & buffer_mask_];
			size_t seq = 
			cell->sequence_.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)pos;
			if (dif == 0)
			{
				if (enqueue_pos_.compare_exchange_weak
					(pos, pos + 1, std::memory_order_relaxed))
				break;
			}
			else if (dif < 0)
				return false;
			else
				pos = enqueue_pos_.load(std::memory_order_relaxed);
		}
		cell->data_ = data;
		cell->sequence_.store(pos + 1, std::memory_order_release);
		return true;
	}

	bool dequeue(T& data)
	{
		cell_t* cell;
		size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
		for (;;)
		{
			cell = &buffer_[pos & buffer_mask_];
			size_t seq = 
				cell->sequence_.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
			if (dif == 0)
			{
				if (dequeue_pos_.compare_exchange_weak
						(pos, pos + 1, std::memory_order_relaxed))
					break;
			}
			else if (dif < 0)
				return false;
			else
				pos = dequeue_pos_.load(std::memory_order_relaxed);
		}
		data = cell->data_;
		cell->sequence_.store
			(pos + buffer_mask_ + 1, std::memory_order_release);
		return true;
	}

private:
	struct cell_t
	{
		std::atomic<size_t>   sequence_;
		T                     data_;
	};

	static size_t const     cacheline_size = 64;
	typedef char            cacheline_pad_t [cacheline_size];

	cacheline_pad_t         pad0_;
	cell_t* const           buffer_;
	size_t const            buffer_mask_;
	cacheline_pad_t         pad1_;
	std::atomic<size_t>     enqueue_pos_;
	cacheline_pad_t         pad2_;
	std::atomic<size_t>     dequeue_pos_;
	cacheline_pad_t         pad3_;

	mpmc_bounded_queue(mpmc_bounded_queue const&);
	void operator = (mpmc_bounded_queue const&);
}; 

struct demand_t
{
	std::size_t m_processed = 0;
	char cache__[ 64 - sizeof(std::size_t) ];
};

using queue_t = mpmc_bounded_queue< demand_t * >;

std::atomic_bool shutdown = ATOMIC_VAR_INIT( false );

void
thread_body( queue_t * queue )
{
	demand_t * ptr;
	while( !shutdown.load( std::memory_order_acquire ) )
		if( queue->dequeue( ptr ) )
		{
			ptr->m_processed += 1;
			queue->enqueue( ptr );
		}
}

void
do_test( unsigned int test_time_seconds, unsigned int thread_count )
{
	queue_t queue { 128 };

	std::vector< demand_t > demands( thread_count, demand_t() );

	std::vector< std::thread > threads;
	threads.reserve( thread_count );
	for( std::size_t i = 0; i != thread_count; ++i )
		threads.emplace_back( thread_body, &queue );

	benchmarker_t benchmarker;
	benchmarker.start();

	for( auto & d : demands )
		queue.enqueue( &d );

	std::this_thread::sleep_for( std::chrono::seconds( test_time_seconds ) );

	shutdown.store( true, std::memory_order_release );

	for( auto & t : threads )
		t.join();

	std::size_t demands_total = std::accumulate( demands.begin(),
			demands.end(), static_cast< std::size_t >(0),
			[]( std::size_t a, const demand_t & b ) { return a + b.m_processed; } );

	benchmarker.finish_and_show_stats( demands_total, "demands" );
}

int main( int argc, char ** argv )
{
	unsigned int seconds = 5;
	unsigned int thread_count = std::thread::hardware_concurrency();

	if( 1 < argc )
		seconds = std::atoi( argv[ 1 ] );
	if( 2 < argc )
		thread_count = std::atoi( argv[ 2 ] );

	std::cout << "seconds: " << seconds << ", threads: " << thread_count
		<< "\n..." << std::endl;

	do_test( seconds, thread_count );
}

