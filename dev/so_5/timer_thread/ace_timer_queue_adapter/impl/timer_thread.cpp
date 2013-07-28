/*
	SObjectizer 5.
*/

#include <algorithm>

#include <so_5/h/log_err.hpp>
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
 * Вспомогательная функция для преобразования миллисекунд в
 * ACE_Time_Value (относительное время).
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
	m_event_handler = std::unique_ptr< timer_event_handler_t >(
		new timer_event_handler_t( *this ) );
}

timer_thread_t::~timer_thread_t()
{
	m_timer_queue.reset();
	m_event_handler.reset();
}

ret_code_t
timer_thread_t::start()
{
	SO_5_ABORT_ON_ACE_ERROR(
		m_timer_queue->activate() );

	return 0;
}

void
timer_thread_t::shutdown()
{
	m_timer_queue->deactivate();
}

void
timer_thread_t::wait()
{
	if( -1 == m_timer_queue->wait() )
		ACE_ERROR( ( LM_ERROR,
					SO_5_LOG_FMT( "%p" ),
					"timer_queue->wait() failed!" ) );

	// Отменяем все события, которые остались.
	cancel_all();
}

timer_id_t
timer_thread_t::schedule_act(
	timer_act_unique_ptr_t & timer_act )
{
	ACE_GUARD_RETURN( ACE_SYNCH_RECURSIVE_MUTEX, lock, mutex(), -1 );

	// Для выставления заявки нужно указать абсолютное время в будущем.
	ACE_Time_Value future_time( ACE_OS::gettimeofday() +
			millisec_to_time_value( timer_act->query_delay() ) );

	timer_id_t msg_id = m_timer_queue->schedule(
			m_event_handler.get(),
			timer_act.get(),
			future_time,
			millisec_to_time_value( timer_act->query_period() ) );

	if( -1 != msg_id )
	{
		// Таймерная заявка успешна принята к обработке.
		// Информация о ней должна быть сохранена.

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
	else
		throw exception_t(
			"unable to schedule delayed or periodic event",
			rc_unable_to_schedule_timer_act );

	return -1;
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

		// Таймерная заявка должна быть уничтожена, т.к. это одиночное
		// отложенное сообщение.
		timer_act_unique_ptr_t timer_act( act );

		// Описание этой таймерной заявки так же должно быть удалено.
		m_scheduled_act_to_id.erase( timer_act.get() );
	}
}

//! Функтор для отмены заявок по их id.
template< class PAIR >
class canceler_by_id_t
{
	public:
		canceler_by_id_t( timer_thread_t & timer_thread )
			:
				m_timer_thread( timer_thread )
		{}

		void
		operator () ( const PAIR & p )
		{
			m_timer_thread.quick_cancel_act( p.second );
		}

	private:
		timer_thread_t & m_timer_thread;
};

void
timer_thread_t::cancel_all()
{
	ACE_GUARD( ACE_SYNCH_RECURSIVE_MUTEX, lock, mutex() );

	std::for_each(
		m_scheduled_act_to_id.begin(),
		m_scheduled_act_to_id.end(),
		canceler_by_id_t< scheduled_act_to_id_map_t::value_type >(
			*this ) );

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
