/*
	SObjectizer 5.
*/

#include <so_5/rt/h/event_exception_handler.hpp>
#include <so_5/rt/h/so_environment.hpp>

namespace so_5
{

namespace rt
{

//
// event_exception_response_action_t
//

event_exception_response_action_t::~event_exception_response_action_t()
{}

//
// event_exception_handler_t
//

event_exception_handler_t::~event_exception_handler_t()
{}

void
event_exception_handler_t::on_install(
	event_exception_handler_unique_ptr_t previous_handler )
{
	// Standard implementation deletes the old handler.
	previous_handler.reset();
}

namespace /* ananymous */
{

//
// std_event_exception_response_action_t
//

//! Standard implementation of the event_exception_response_action.
/*!
 * Deregisters the cooperation with failed agent.
 */
class std_event_exception_response_action_t
	:
		public event_exception_response_action_t
{
	public:
		std_event_exception_response_action_t(
			so_environment_t & so_environment,
			const std::string & coop_name )
			:
				m_so_environment( so_environment ),
				m_coop_name( coop_name )
		{}

		virtual ~std_event_exception_response_action_t()
		{}

		virtual void
		respond_to_exception()
		{
			m_so_environment.deregister_coop( m_coop_name );
		}

	private:
		so_environment_t & m_so_environment;
		const std::string m_coop_name;
};


//
// std_event_exception_handler_t
//

//! Standard exception handler.
/*!
 * Deregisters the cooperation with failed agent.
 */
class std_event_exception_handler_t
	:
		public event_exception_handler_t
{
	public:
		virtual ~std_event_exception_handler_t()
		{}

		event_exception_response_action_unique_ptr_t
		handle_exception(
			so_environment_t & so_environment,
			const std::exception & event_exception,
			const std::string & coop_name )
		{
			return event_exception_response_action_unique_ptr_t(
				new std_event_exception_response_action_t(
					so_environment,
					coop_name ) );
		}
};

} /* ananymous namespace */

//
// create_std_event_exception_handler
//

CPP_UTIL_2_EXPORT_FUNC_SPEC( event_exception_handler_unique_ptr_t )
create_std_event_exception_handler()
{
	return event_exception_handler_unique_ptr_t(
		new std_event_exception_handler_t );
}

} /* namespace rt */

} /* namespace so_5 */

