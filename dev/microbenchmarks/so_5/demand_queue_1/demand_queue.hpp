#if !defined( DEMAND_QUEUE_1__DEMAND_QUEUE_HPP )
#define DEMAND_QUEUE_1__DEMAND_QUEUE_HPP

#include <deque>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Atomic_Op.h>
#include <ace/Guard_T.h>
#include <ace/Thread_Manager.h>

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

		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_not_empty;

		bool m_shutting_down;
		bool m_is_waiting;
};

demand_queue_t::demand_queue_t()
	:	m_not_empty( m_lock )
	,	m_shutting_down( false )
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
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

	if( !m_shutting_down )
	{
		m_demands.push_back( demand_t( handler, param ) );

		if( m_is_waiting )
		{
			// Someone is waiting...
			// It should be informed about new demands.
			m_is_waiting = false;
			m_not_empty.signal();
		}
	}
}

int
demand_queue_t::pop(
	demand_container_t & demands )
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

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
			m_not_empty.wait();
		}
	}

	return demand_extracted;
}

void
demand_queue_t::start_service()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
}

void
demand_queue_t::stop_service()
{
	{
		ACE_Guard< ACE_Thread_Mutex > lock( m_lock );

		m_shutting_down = true;

		// If the demands queue is empty then someone is waiting
		// for new demands inside pop().
		if( m_is_waiting )
			m_not_empty.signal();
	}
}

void
demand_queue_t::clear()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
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

		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

	private:
		demand_queue_t m_queue;

		enum
		{
			WORK_THREAD_STOP = 0,
			WORK_THREAD_CONTINUE = 1
		};

		ACE_Atomic_Op< ACE_Thread_Mutex, long > m_continue_work;

		ACE_thread_t m_tid;
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
	m_continue_work = WORK_THREAD_CONTINUE;

	if( -1 == ACE_Thread_Manager::instance()->spawn(
			entry_point,
			this,
			THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
			&m_tid ) )
	{
		ACE_ERROR(
				(LM_EMERGENCY,
				"work_thread_t::start(): call of "
				"ACE_Thread_Manager::instance()->spawn() failed, "
				"last_error: %p\n") );

		ACE_OS::abort();
	}
}

void
work_thread_t::shutdown()
{
	m_continue_work = WORK_THREAD_STOP;
	m_queue.stop_service();
}

void
work_thread_t::wait()
{
	ACE_Thread_Manager::instance()->join( m_tid );

	m_queue.clear();
}

void
work_thread_t::body()
{
	// Local demands queue.
	demand_container_t demands;

	int result = demand_queue_t::no_demands;

	while( m_continue_work.value() == WORK_THREAD_CONTINUE )
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

ACE_THR_FUNC_RETURN
work_thread_t::entry_point( void * self_object )
{
	work_thread_t * work_thread =
		reinterpret_cast<work_thread_t *>( self_object );

	work_thread->body();

	return 0;
}

#endif

