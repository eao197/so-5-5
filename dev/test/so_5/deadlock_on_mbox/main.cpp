#include <iostream>

#include <ace/Task_T.h>
#include <ace/RW_Thread_Mutex.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

class test_controller_t
	{
	public :
		test_controller_t()
			:	m_subscribe_event_handler_point( m_lock )
			,	m_push_event_point( m_lock )
			{}

#if 0
		void
		pause_for_start()
			{
				m_lock.acquire();
			}

		void
		do_start()
			{
				m_start_point.broadcast();
				m_lock.release();
			}

		void
		wait_for_start_point()
			{
			}
#endif

		void
		wait_for_subscribe_event_handler_point()
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				m_subscribe_event_handler_point.wait();
			}

		void
		subscribe_event_handler_point_reached()
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				m_subscribe_event_handler_point.signal();
			}

		void
		wait_for_push_event_point()
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				m_push_event_point.wait();
			}

		void
		push_event_point_reached()
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				m_push_event_point.signal();
			}

	private :
		ACE_Thread_Mutex m_lock;
//		ACE_Condition_Thread_Mutex m_start_point;
		ACE_Condition_Thread_Mutex m_subscribe_event_handler_point;
		ACE_Condition_Thread_Mutex m_push_event_point;
	};

class mbox_iface_t
	{
	public :
		virtual void
		subscribe() = 0;
	};

class agent_iface_t
	{
	public :
		virtual void
		push_event() = 0;
	};

class first_thread_t
	:	public ACE_Task< ACE_MT_SYNCH >
	,	public agent_iface_t
	{
	public :
		first_thread_t(
			test_controller_t & controller )
			:	m_controller( controller )
			,	m_mbox( nullptr )
			{}

		void
		bind_mbox( mbox_iface_t & mbox )
			{
				m_mbox = &mbox;
			}

		virtual int
		svc()
			{
				std::cout << "T1: acquiring A.lock" << std::endl;

				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				std::cout << "T1: A.lock acquired" << std::endl;

				std::cout << "T1: subscribe_event_handler point reached" << std::endl;
				m_controller.subscribe_event_handler_point_reached();

				std::cout << "T1: wait for push_event point" << std::endl;
				m_controller.wait_for_push_event_point();

				std::cout << "T1: calling M.subscribe" << std::endl;
				m_mbox->subscribe();

				std::cout << "T1: M.subscribe passed" << std::endl;

				return 0;
			}

		virtual void
		push_event()
			{
				std::cout << "T2: push_event enter" << std::endl;

				ACE_Guard< ACE_Thread_Mutex > lock( m_lock );
				std::cout << "T2: A.lock acquired" << std::endl;

				std::cout << "T2: push_event leave" << std::endl;
			}

	private :
		test_controller_t & m_controller;
		mbox_iface_t * m_mbox;

		ACE_Thread_Mutex m_lock;
	};

class second_thread_t
	:	public ACE_Task< ACE_MT_SYNCH >
	,	public mbox_iface_t
	{
	public :
		second_thread_t(
			test_controller_t & controller )
			:	m_controller( controller )
			,	m_agent( nullptr )
			{}

		void
		bind_agent( agent_iface_t & agent )
			{
				m_agent = &agent;
			}

		virtual int
		svc()
			{
				m_controller.wait_for_subscribe_event_handler_point();

				std::cout << "T2: acquiring M.lock read-only" << std::endl;

				ACE_Read_Guard< ACE_RW_Thread_Mutex > read_lock( m_lock );
				std::cout << "T2: M.lock read-only locked" << std::endl;

				std::cout << "T2: starting event pushing" << std::endl;
				m_controller.push_event_point_reached();

				m_agent->push_event();

				std::cout << "T2: event pushing finished" << std::endl;

				return 0;
			}

		virtual void
		subscribe()
			{
				std::cout << "T1: M.subscribe enter" << std::endl;

				ACE_Write_Guard< ACE_RW_Thread_Mutex > write_lock( m_lock );
				std::cout << "T1: M.lock read-write locked" << std::endl;

				std::cout << "T1: M.subscribe leave" << std::endl;
			}

	private :
		test_controller_t & m_controller;
		agent_iface_t * m_agent;

		ACE_RW_Thread_Mutex m_lock;
	};

int
main( int argc, char ** argv )
	{
		test_controller_t controller;

		first_thread_t first( controller );
		second_thread_t second( controller );

		first.bind_mbox( second );
		second.bind_agent( first );

		first.activate();
		second.activate();

		first.wait();
		second.wait();

		return 0;
	}

