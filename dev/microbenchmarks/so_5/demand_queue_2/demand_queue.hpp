#if !defined( DEMAND_QUEUE_2__DEMAND_QUEUE_HPP )
#define DEMAND_QUEUE_2__DEMAND_QUEUE_HPP

#include <deque>
#include <queue>
#include <list>

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Atomic_Op.h>
#include <ace/Guard_T.h>
#include <ace/Thread_Manager.h>

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

struct agent_demand_t
	{
		message_t * m_msg;
		event_caller_t * m_caller;

		agent_demand_t()
			:	m_msg( nullptr )
			,	m_caller( nullptr )
			{}

		agent_demand_t(
			message_t * msg,
			event_caller_t * caller )
			:	m_msg( msg )
			,	m_caller( caller )
			{}
	};

class agent_demand_storage_t
	{
	public :
		virtual ~agent_demand_storage_t()
			{}

		virtual void
		push( message_t * msg, event_caller_t * caller ) = 0;

		virtual agent_demand_t
		pop() = 0;
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
		push( agent_t * agent ) = 0;

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
			push( agent_t * agent )
				{
					m_notificator.push( agent );
				}
		};
	};

class agent_t
	{
	private :
		std::unique_ptr< agent_demand_storage_t > m_demand_storage;

		dispatcher_notificator_t & m_dispatcher_notificator;

	public :
		agent_t(
			std::unique_ptr< agent_demand_storage_t > demand_storage,
			dispatcher_notificator_t & dispatcher_notificator )
			:	m_demand_storage( std::move( demand_storage ) )
			,	m_dispatcher_notificator( dispatcher_notificator )
			{}
		virtual ~agent_t()
			{}

		agent_demand_storage_t &
		demand_storage()
			{
				return *m_demand_storage;
			}

		dispatcher_notificator_t &
		dispatcher_notificator()
			{
				return m_dispatcher_notificator;
			}
	};

class simple_demand_storage_t
	:	public agent_demand_storage_t
	{
		typedef std::deque< agent_demand_t > container_t;

		container_t m_storage;

	public :
		virtual ~simple_demand_storage_t()
			{}

		virtual void
		push( message_t * msg, event_caller_t * caller )
			{
				m_storage.push_back( agent_demand_t( msg, caller ) );
			}

		virtual agent_demand_t
		pop()
			{
				auto result = m_storage.front();
				m_storage.pop_front();
				return result;
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
		typedef std::queue< agent_t * > notifies_t;

		notifies_t m_notifies;

		ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_condition;

		bool m_shutting_down;
		bool m_someone_waiting;

	public :
		real_dispatcher_notificator_t()
			:	m_condition( m_lock )
			,	m_shutting_down( false )
			,	m_someone_waiting( false )
			{}

		virtual ~real_dispatcher_notificator_t()
			{}

		event_info_t
		wait_and_pop()
			{
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
						agent_t * a = m_notifies.front();
						m_notifies.pop();

						const agent_demand_t & d = a->demand_storage().pop();

						return event_info_t(
								a,
								d.m_msg,
								d.m_caller );
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
		push( agent_t * agent )
			{
				m_notifies.push( agent );

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

