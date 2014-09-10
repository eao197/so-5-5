/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.5.0
	\brief Timers and tools for working with timers.
*/

#include <so_5/h/timers.hpp>

#include <timertt/all.hpp>

namespace so_5
{

//
// timer_t
//
timer_t::~timer_t()
	{}

//
// timer_id_t
//
timer_id_t::timer_id_t()
	{}

timer_id_t::timer_id_t(
	so_5::rt::smart_atomic_reference_t< timer_t > && timer )
	:	m_timer( std::move( timer ) )
	{}

timer_id_t::timer_id_t(
	const timer_id_t & o )
	:	m_timer( o.m_timer )
	{}

timer_id_t::timer_id_t(
	timer_id_t && o )
	:	m_timer( std::move( o.m_timer ) )
	{}

timer_id_t::~timer_id_t()
	{}

timer_id_t &
timer_id_t::operator=( const timer_id_t & o )
	{
		timer_id_t t( o );
		t.swap( *this );
		return *this;
	}

timer_id_t &
timer_id_t::operator=( timer_id_t && o )
	{
		timer_id_t t( std::move( o ) );
		t.swap( *this );
		return *this;
	}

void
timer_id_t::swap( timer_id_t & o )
	{
		m_timer.swap( o.m_timer );
	}

bool
timer_id_t::is_active() const
	{
		return ( m_timer && m_timer->is_active() );
	}

void
timer_id_t::release()
	{
		if( m_timer )
			m_timer->release();
	}

//
// timer_thread_t
//

timer_thread_t::timer_thread_t()
	{}

timer_thread_t::~timer_thread_t()
	{}

//FIXME: this namespace must be described in namespaces.dox
namespace timers_details
{

//
// actual_timer_t
//
/*!
 * \since v.5.5.0
 * \brief An actual implementation of timer interface.
 * 
 * \tparam TIMER_THREAD A type of timertt-based thread which implements timers.
 */
template< class TIMER_THREAD >
class actual_timer_t : public timer_t
	{
	public :
		//! Initialized constructor.
		actual_timer_t(
			TIMER_THREAD * thread )
			:	m_thread( thread )
			,	m_timer( thread->allocate() )
			{}
		virtual ~actual_timer_t()
			{
				release();
			}

		timertt::timer_holder_t &
		timer_holder()
			{
				return m_timer;
			}

		virtual bool
		is_active() const override
			{
				return (m_thread != nullptr);
			}

		virtual void
		release() override
			{
				if( m_thread )
				{
					m_thread->deactivate( m_timer );
					m_thread = nullptr;
					m_timer.reset();
				}
			}

	private :
		//! Timer thread for the timer.
		/*!
		 * nullptr means that timer is deactivated.
		 */
		TIMER_THREAD * m_thread;

		//! Underlying timer object reference.
		timertt::timer_holder_t m_timer;
	};

//
// actual_thread_t
//
/*!
 * \since v.5.5.0
 * \brief An actual implementation of timer thread.
 * 
 * \tparam TIMER_THREAD A type of timertt-based thread which implements timers.
 */
template< class TIMER_THREAD >
class actual_thread_t : public timer_thread_t
	{
		typedef actual_timer_t< TIMER_THREAD > actual_timer_t;

	public :
		//! Initializing constructor.
		actual_thread_t(
			//! Real timer thread.
			std::unique_ptr< TIMER_THREAD > thread )
			:	m_thread( std::move( thread ) )
			{}

		virtual void
		start() override
			{
				m_thread->start();
			}

		virtual void
		finish() override
			{
				m_thread->shutdown_and_join();
			}

		virtual timer_id_t
		schedule(
			const std::type_index & type_index,
			const so_5::rt::mbox_ref_t & mbox_r,
			const so_5::rt::message_ref_t & msg_r,
			std::chrono::milliseconds pause,
			std::chrono::milliseconds period ) override
			{
				std::unique_ptr< actual_timer_t > timer(
						new actual_timer_t( m_thread.get() ) );

				so_5::rt::mbox_ref_t mbox{ mbox_r };
				so_5::rt::message_ref_t msg{ msg_r };
				m_thread->activate( timer->timer_holder(),
						pause,
						period,
						[type_index, mbox, msg]()
						{
							mbox->deliver_message( type_index, msg );
						} );

				return timer_id_t( timer.release() );
			}

		virtual void
		schedule_anonymous(
			const std::type_index & type_index,
			const so_5::rt::mbox_ref_t & mbox,
			const so_5::rt::message_ref_t & msg,
			std::chrono::milliseconds pause,
			std::chrono::milliseconds period ) override
			{
				m_thread->activate(
						pause,
						period,
						[type_index, mbox, msg]()
						{
							mbox->deliver_message( type_index, msg );
						} );
			}

	private :
		std::unique_ptr< TIMER_THREAD > m_thread;
	};

} /* namespace timers_details */

SO_5_EXPORT_FUNC_SPEC( timer_thread_unique_ptr_t )
create_timer_wheel_thread()
	{
		using namespace timers_details;
		using timertt_thread_t = timertt::timer_wheel_thread_t;

		std::unique_ptr< timertt_thread_t > thread( new timertt_thread_t() );

		return timer_thread_unique_ptr_t(
				new actual_thread_t< timertt_thread_t >( std::move( thread ) ) );
	}

SO_5_EXPORT_FUNC_SPEC( timer_thread_unique_ptr_t )
create_timer_list_thread()
	{
//FIXME: implement this!
		return timer_thread_unique_ptr_t();
	}

} /* namespace so_5 */

