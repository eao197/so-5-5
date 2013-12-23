#if !defined( DEMAND_QUEUE_1__DEMAND_QUEUE_HPP )
#define DEMAND_QUEUE_1__DEMAND_QUEUE_HPP

#include <deque>
#include <atomic>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <thread>

typedef void (*handler_pfn_t)(void *);

//
// demand_t
//

struct demand_t
{
	handler_pfn_t m_handler;
	void * m_param;

	demand_t()
		:	m_handler( nullptr )
		,	m_param( nullptr )
	{}

	demand_t(
		handler_pfn_t handler,
		void * param )
		:	m_handler( handler )
		,	m_param( param )
	{}
};

typedef std::deque< demand_t > demand_container_t;

//
// demand_queue_t
//

class demand_queue_t
{
	public:
		demand_queue_t();
		~demand_queue_t();

		void
		push(
			handler_pfn_t handler,
			void * param );

		enum
		{
			demand_extracted = 1,
			shutting_down = 2,
			no_demands = 3
		};

		int
		pop(
			demand_container_t & queue_item );

		void
		start_service();

		void
		stop_service();

		void
		clear();

	private:
		demand_container_t m_demands;

		std::mutex m_lock;
		std::condition_variable m_not_empty;

		bool m_shutting_down;
		bool m_is_waiting;
};

demand_queue_t::demand_queue_t()
	:	m_shutting_down( false )
	,	m_is_waiting( false )
{
}

demand_queue_t::~demand_queue_t()
{
	m_demands.clear();
}

void
demand_queue_t::push(
	handler_pfn_t handler,
	void * param )
{
	std::unique_lock< std::mutex > lock( m_lock );

	if( !m_shutting_down )
	{
		m_demands.push_back( demand_t( handler, param ) );

		if( m_is_waiting )
		{
			// Someone is waiting...
			// It should be informed about new demands.
			m_is_waiting = false;
			m_not_empty.notify_one();
		}
	}
}

int
demand_queue_t::pop(
	demand_container_t & demands )
{
	std::unique_lock< std::mutex > lock( m_lock );

	while( true )
	{
		if( m_shutting_down )
			return shutting_down;
		else if( !m_demands.empty() )
		{
			demands.swap( m_demands );
			break;
		}
		else
		{
			// Queue is empty. We should wait for a demand or
			// a shutdown signal.
			m_is_waiting = true;
			m_not_empty.wait( lock );
		}
	}

	return demand_extracted;
}

void
demand_queue_t::start_service()
{
}

void
demand_queue_t::stop_service()
{
	std::unique_lock< std::mutex > lock( m_lock );

	m_shutting_down = true;

	// If the demands queue is empty then someone is waiting
	// for new demands inside pop().
	if( m_is_waiting )
		m_not_empty.notify_one();
}

void
demand_queue_t::clear()
{
	std::unique_lock< std::mutex > lock( m_lock );
	m_demands.clear();
}

//
// work_thread_t
//

class work_thread_t
{
	public:
		work_thread_t();
		~work_thread_t();

		void
		put_event(
			handler_pfn_t handler,
			void * param );

		void
		start();

		void
		shutdown();

		void
		wait();

	protected:
		void
		body();

		void
		serve_demands_block(
			demand_container_t & executed_demands );

		static void
		entry_point( work_thread_t * self_object );

	private:
		demand_queue_t m_queue;

		std::atomic< bool > m_continue;

		std::unique_ptr< std::thread > m_thread;
};

work_thread_t::work_thread_t()
{
}

work_thread_t::~work_thread_t()
{
}

void
work_thread_t::put_event(
	handler_pfn_t handler,
	void * param )
{
	m_queue.push( handler, param );
}

void
work_thread_t::start()
{
	m_queue.start_service();
	m_continue = true;

	m_thread.reset(
			new std::thread( &work_thread_t::entry_point, this ) );
}

void
work_thread_t::shutdown()
{
	m_continue = false;
	m_queue.stop_service();
}

void
work_thread_t::wait()
{
	m_thread->join();

	m_queue.clear();
}

void
work_thread_t::body()
{
	// Local demands queue.
	demand_container_t demands;

	int result = demand_queue_t::no_demands;

	while( m_continue )
	{
		if( demands.empty() )
			result = m_queue.pop( demands );

		// Serve demands if any.
		if( demand_queue_t::demand_extracted == result )
			serve_demands_block( demands );
	}
}

inline void
work_thread_t::serve_demands_block(
	demand_container_t & demands )
{
	while( !demands.empty() )
	{
		demand_t demand = demands.front();
		demands.pop_front();

		(*demand.m_handler)(demand.m_param);
	}
}

void
work_thread_t::entry_point( work_thread_t * self_object )
{
	self_object->body();
}

#endif

