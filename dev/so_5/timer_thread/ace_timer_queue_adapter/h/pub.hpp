/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Function for creating timer_thread implementation.
*/

#if !defined( _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__PUB_HPP_ )
#define _SO_5__TIMER_THREAD__ACE_TIMER_QUEUE_ADAPTER__PUB_HPP_

#include <so_5/h/declspec.hpp>

#include <so_5/timer_thread/h/timer_thread.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

/*!
	\brief Create timer_thread implementation.
 */
SO_5_EXPORT_FUNC_SPEC( so_5::timer_thread::timer_thread_unique_ptr_t )
create_timer_thread();

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */

#endif

