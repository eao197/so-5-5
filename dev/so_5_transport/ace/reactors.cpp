/*
	SObjectizer 5 Transport.
*/

#include <so_5/h/log_err.hpp>

#include <so_5_transport/ace/h/reactors.hpp>

#include <ace/Singleton.h>
#include <ace/Task.h>
#include <ace/Thread.h>
#include <ace/Select_Reactor.h>
#include <ace/TP_Reactor.h>

namespace so_5_transport
{

namespace ace
{

//
// reactor_instance_t
//

reactor_instance_t::~reactor_instance_t()
{
}

//
// make_select_reactor
//

SO_5_TRANSPORT_EXPORT_FUNC_SPEC( reactor_instance_unique_ptr_t )
make_select_reactor()
{
	return reactor_instance_unique_ptr_t(
		new reactor_instance_template_t< ACE_Select_Reactor >( 1 ) );
}

//
// make_tp_reactor
//

SO_5_TRANSPORT_EXPORT_FUNC_SPEC( reactor_instance_unique_ptr_t )
make_tp_reactor(
	size_t thread_count )
{
	return reactor_instance_unique_ptr_t(
		new reactor_instance_template_t< ACE_TP_Reactor >(
			thread_count ) );
}

//
// end_reactor_event_loop_call_handler_t
//

end_reactor_event_loop_call_handler_t::end_reactor_event_loop_call_handler_t(
	ACE_Reactor * r )
	:
		ACE_Event_Handler( r )
{
}

int
end_reactor_event_loop_call_handler_t::handle_exception( ACE_HANDLE )
{
	reactor()->end_reactor_event_loop();
	return -1;
}

int
end_reactor_event_loop_call_handler_t::handle_close(
	ACE_HANDLE ,
	ACE_Reactor_Mask )
{
	delete this;
	return 0;
}

end_reactor_event_loop_call_handler_t::~end_reactor_event_loop_call_handler_t()
{
}

} /* namespace ace */

} /* namespace so_5_transport */

