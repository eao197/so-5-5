/*
	SObjectizer 5.
*/

#include <so_5/timer_thread/ace_timer_queue_adapter/h/pub.hpp>

#include <so_5/timer_thread/ace_timer_queue_adapter/impl/h/timer_thread.hpp>

namespace so_5
{

namespace timer_thread
{

namespace ace_timer_queue_adapter
{

SO_5_EXPORT_FUNC_SPEC( so_5::timer_thread::timer_thread_unique_ptr_t )
create_timer_thread()
{
	return timer_thread_unique_ptr_t( new impl::timer_thread_t() );
}

} /* namespace ace_timer_queue_adapter */

} /* namespace timer_thread */

} /* namespace so_5 */
