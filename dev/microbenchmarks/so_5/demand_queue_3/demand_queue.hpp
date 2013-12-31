#if !defined( DEMAND_QUEUE_3__DEMAND_QUEUE_HPP )
#define DEMAND_QUEUE_3__DEMAND_QUEUE_HPP

#include <deque>
#include <queue>
#include <list>
#include <vector>
#include <cstdint>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Atomic_Op.h>
#include <ace/Guard_T.h>
#include <ace/Thread_Manager.h>

template< class T >
class circular_buffer_queue_t
	{
		typedef std::vector< T > buffer_t;

		const size_t m_buffer_size;

		buffer_t m_buffer;

		uint64_t m_head_index;
		uint64_t m_tail_index;

		inline size_t
		actual_position( uint64_t index ) const
			{
				return index % m_buffer_size;
			}

	public :
		circular_buffer_queue_t(
			size_t buffer_size )
			:	m_buffer_size( buffer_size )
			,	m_buffer( buffer_size )
			,	m_head_index( 0 )
			,	m_tail_index( 0 )
			{}

		bool
		empty() const
			{
				return m_head_index == m_tail_index;
			}

		T &
		front()
			{
				return m_buffer[ actual_position( m_head_index ) ];
			}

		void
		pop()
			{
				++m_head_index;
			}

		void
		pop_front()
			{
				pop();
			}

		void
		push( const T & item )
			{
				m_buffer[ actual_position( m_tail_index ) ] = item;
				++m_tail_index;
			}

		void
		push_back( const T & item )
			{
				push( item );
			}
	};

class agent_t;

class message_t
	{
	public :
		virtual ~message_t()
			{}
	};

class event_caller_t
	{
	public :
		virtual void
		call( agent_t * agent, message_t * msg ) = 0;
	};

struct demand_t
	{
		agent_t * m_agent;
		message_t * m_msg;
		event_caller_t * m_caller;

		demand_t()
			:	m_agent( nullptr )
			,	m_msg( nullptr )
			,	m_caller( nullptr )
			{}

		demand_t(
			agent_t * agent,
			message_t * msg,
			event_caller_t * caller )
			:	m_agent( agent )
			,	m_msg( msg )
			,	m_caller( caller )
			{}
	};

class dispatcher_notificator_t
	{
		class local_lock_t;
		friend class dispatcher_notificator_t::local_lock_t;

	private :
		virtual void
		lock() = 0;

		virtual void
		unlock() = 0;

		virtual void
		push( agent_t * agent, message_t * msg, event_caller_t * caller ) = 0;

	public :
		class local_lock_t
		{
			dispatcher_notificator_t & m_notificator;
		public :
			local_lock_t( dispatcher_notificator_t & notificator )
				:	m_notificator( notificator )
				{
					m_notificator.lock();
				}
			~local_lock_t()
				{
					m_notificator.unlock();
				}

			void
			push( agent_t * agent,
				message_t * msg,
				event_caller_t * caller )
				{
					m_notificator.push( agent, msg, caller );
				}
		};
	};

class agent_t
	{
	private :
		dispatcher_notificator_t & m_dispatcher_notificator;

	public :
		agent_t(
			dispatcher_notificator_t & dispatcher_notificator )
			:	m_dispatcher_notificator( dispatcher_notificator )
			{}
		virtual ~agent_t()
			{}

		dispatcher_notificator_t &
		dispatcher_notificator()
			{
				return m_dispatcher_notificator;
			}

		void
		send_message( message_t * msg, event_caller_t & caller )
			{
				dispatcher_notificator_t::local_lock_t lock(
						dispatcher_notificator() );

				lock.push( this, msg, &caller );
			}
	};

class real_dispatcher_notificator_t
	:	public dispatcher_notificator_t
	{
	public :
		struct event_info_t
			{
				agent_t * m_agent;
				message_t * m_msg;
				event_caller_t * m_caller;

				event_info_t()
					:	m_agent( nullptr )
					,	m_msg( nullptr )
					,	m_caller( nullptr )
					{}

				event_info_t(
					agent_t * agent,
					message_t * msg,
					event_caller_t * caller )
					:	m_agent( agent )
					,	m_msg( msg )
					,	m_caller( caller )
					{}

				bool
				empty() const
					{
						return m_agent == nullptr &&
								m_msg == nullptr && m_caller == nullptr;
					}
			};

	private :
		typedef circular_buffer_queue_t< event_info_t > notifies_t;

		notifies_t m_notifies;

		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_condition;

		ACE_Atomic_Op< ACE_Thread_Mutex, unsigned long > m_not_empty;

		bool m_shutting_down;
		bool m_someone_waiting;

	public :
		real_dispatcher_notificator_t()
			:	m_notifies( 16 )
			,	m_condition( m_lock )
			,	m_not_empty( 0 )
			,	m_shutting_down( false )
			,	m_someone_waiting( false )
			{}

		virtual ~real_dispatcher_notificator_t()
			{}

		event_info_t
		wait_and_pop()
			{
				if( 0 == m_not_empty.value() )
					SwitchToThread();

				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				do
				{
					if( m_shutting_down )
						break;

					if( m_notifies.empty() )
					{
						m_someone_waiting = true;
						m_condition.wait();
						m_someone_waiting = false;
					}
					else
					{
						event_info_t info = m_notifies.front();
						m_notifies.pop();

						m_not_empty = m_notifies.empty() ? 0 : 1;

						return info;
					}
				}
				while( true );

				return event_info_t();
			}

		void
		shutdown()
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				
				m_shutting_down = true;
				if( m_someone_waiting )
					m_condition.signal();
			}

	private :
		virtual void
		lock()
			{
				m_lock.acquire();
			}

		virtual void
		unlock()
			{
				m_lock.release();
			}

		virtual void
		push( agent_t * agent, message_t * msg, event_caller_t * caller )
			{
				m_notifies.push( event_info_t( agent, msg, caller ) );
				m_not_empty = 1;

				if( m_someone_waiting )
					m_condition.signal();
			}
	};

//
// work_thread_t
//

class work_thread_t
	{
	public:
		work_thread_t();
		~work_thread_t();

		dispatcher_notificator_t &
		dispatcher_notificator();

		void
		start();

		void
		shutdown();

		void
		wait();

	protected:
		void
		body();

		static ACE_THR_FUNC_RETURN
		entry_point( void * self_object );

	private:
		real_dispatcher_notificator_t m_dispatcher_notificator;

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

dispatcher_notificator_t &
work_thread_t::dispatcher_notificator()
	{
		return m_dispatcher_notificator;
	}

void
work_thread_t::start()
	{
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
		m_dispatcher_notificator.shutdown();
	}

void
work_thread_t::wait()
	{
		ACE_Thread_Manager::instance()->join( m_tid );
	}

void
work_thread_t::body()
	{
		while( m_continue_work.value() == WORK_THREAD_CONTINUE )
		{
			const real_dispatcher_notificator_t::event_info_t & e =
					m_dispatcher_notificator.wait_and_pop();
			if( e.empty() )
				break;

			std::unique_ptr< message_t > msg_destroyer( e.m_msg );
			e.m_caller->call( e.m_agent, e.m_msg );
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

