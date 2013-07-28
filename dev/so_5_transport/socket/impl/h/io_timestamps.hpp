/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Временные отметки чтения/записи.
*/

#if !defined( _SO_5_TRANSPORT__SOCKET__IMPL__IO_TIMESTAMPS_HPP_ )
#define _SO_5_TRANSPORT__SOCKET__IMPL__IO_TIMESTAMPS_HPP_

#include <ace/Time_Value.h>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// io_timestamps_t
//

//! Временные пометки операций чтения/записи.
class io_timestamps_t
{
	public:
		io_timestamps_t();

		//! Отметить время последнего чтения.
		void
		mark_last_input();

		//! Отметить время последней записи.
		void
		mark_last_output();

		//! Сколько времени прошло с момента последнего чтения.
		ACE_Time_Value
		since_last_input() const;

		//! Сколько времени прошло с момента последней записи.
		ACE_Time_Value
		since_last_output() const;

	private:
		//! Последнее время чтения.
		ACE_Time_Value m_last_input;

		//! Последнее время записи.
		ACE_Time_Value m_last_output;
};

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

#endif
