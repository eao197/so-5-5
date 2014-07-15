/*
	SObjectizer 5.
*/

#include <algorithm>

#include <cpp_util_2/h/lexcast.hpp>

#include <so_5/h/exception.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/timer_thread/ace_timer_queue_adapter/impl/h/timer_thread.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

namespace impl
{

namespace
{

/*!
 * Auxiliary fuction for the conversion from milliseconds to
 * ACE_Time_Value (in relative format).
 */
inline ACE_Time_Value
millisec_to_time_value( unsigned int m )
{
	return ACE_Time_Value( m / 1000, ( m % 1000 ) * 1000 );
}

} /* namespace anonymous */


//
// timer_thread_t
//

timer_thread_t::timer_thread_t()
	:
		m_timer_queue( new adapter_t() ),
		m_self_id_counter( 0 )
{
	m_event_handler.reset( new timer_event_handler_t( *this ) );
}

timer_thread_t::~timer_thread_t()
{
	// Detructor for the timer_queue must be called before
	// destructor of event_handler!
	m_timer_queue.reset();
	m_event_handler.reset();
}

void
timer_thread_t::start()
{
	if( -1 == m_timer_queue->activate() )
	{
		SO_5_THROW_EXCEPTION(
				rc_unexpected_error,
				"timer_thread_t::start(): call of "
				"m_timer_queue->activate() failed, last_error: " +
				cpp_util_2::slexcast( ACE_OS::last_error() ) );
	}
}

void
timer_thread_t::finish()
{
	m_timer_queue->deactivate();

	if( -1 == m_timer_queue->wait() )
	{
		SO_5_THROW_EXCEPTION(
				rc_unexpected_error,
				"timer_thread_t::finish(): call of "
				"m_timer_queue->wait() failed, last_error: " +
				cpp_util_2::slexcast( ACE_OS::last_error() ) );
	}

	cancel_all();
}

timer_id_t
timer_thread_t::schedule_act(
	timer_act_unique_ptr_t timer_act )
{
	ACE_Guard< ACE_SYNCH_RECURSIVE_MUTEX > lock( mutex() );

	// Absolute time in the future should be used.
	ACE_Time_Value future_time( ACE_OS::gettimeofday() +
			millisec_to_time_value( timer_act->query_delay() ) );

	auto msg_id = m_timer_queue->schedule(
			m_event_handler.get(),
			timer_act.get(),
			future_time,
			millisec_to_time_value( timer_act->query_period() ) );

	if( -1 == msg_id )
		throw exception_t(
			"unable to schedule delayed or periodic event",
			rc_unable_to_schedule_timer_act );

	// Timer event was successfully registered.
	// Information about it should be stored.

	timer_keys_t timer_keys( ++m_self_id_counter, msg_id );

	m_scheduled_act_to_id.insert(
		scheduled_act_to_id_map_t::value_type(
			timer_act.get(),
			timer_keys ) );

	m_timer_id_to_ace_id.insert(
		timer_id_to_ace_id_map_t::value_type(
			timer_keys.m_timer_id,
			timer_keys.m_ace_id ) );

	timer_act.release();
	return timer_keys.m_timer_id;
}

void
timer_thread_t::cancel_act(
	timer_id_t msg_id )
{
	const void * raw_act;

	ACE_GUARD( ACE_SYNCH_RECURSIVE_MUTEX, lock, mutex() );

	timer_id_to_ace_id_map_t::iterator it =
		m_timer_id_to_ace_id.find( msg_id );
	if( m_timer_id_to_ace_id.end() != it )
	{
		if( 0 != m_timer_queue->cancel( it->second , &raw_act ) )
		{
			timer_act_unique_ptr_t timer_act(
				reinterpret_cast< timer_act_t * >(
					const_cast< void * >( raw_act ) ) );

			m_scheduled_act_to_id.erase( timer_act.get() );
		}
		m_timer_id_to_ace_id.erase( it );
	}
}

void
timer_thread_t::quick_cancel_act(
	const timer_thread_t::timer_keys_t & timer_keys )
{
	const void * raw_act;

	if( 0 != m_timer_queue->cancel( timer_keys.m_ace_id, &raw_act ) )
	{
		delete reinterpret_cast< timer_act_t * >(
			const_cast< void * >( raw_act ) );
	}
}

void
timer_thread_t::utilize_timer_act(
	timer_act_t * act )
{
	if( !act->is_periodic() )
	{
		ACE_GUARD( ACE_SYNCH_RECURSIVE_MUTEX, lock, mutex() );

		// Timer event should be destroyed because it is not an
		// periodic message.
		timer_act_unique_ptr_t timer_act( act );

		// Its description should be destroyed too.
		auto it = m_scheduled_act_to_id.find( timer_act.get() );
		if( m_scheduled_act_to_id.end() != it )
		{
			m_timer_id_to_ace_id.erase( it->second.m_timer_id );
			m_scheduled_act_to_id.erase( it );
		}
	}
}

void
timer_thread_t::cancel_all()
{
	ACE_GUARD( ACE_SYNCH_RECURSIVE_MUTEX, lock, mutex() );

	std::for_each(
		m_scheduled_act_to_id.begin(),
		m_scheduled_act_to_id.end(),
		[this]( scheduled_act_to_id_map_t::value_type & a ) {
			this->quick_cancel_act( a.second );
		} );

	m_timer_id_to_ace_id.clear();
	m_scheduled_act_to_id.clear();
}

ACE_SYNCH_RECURSIVE_MUTEX &
timer_thread_t::mutex()
{
	return m_timer_queue->mutex();
}

} /* namespace impl */

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

