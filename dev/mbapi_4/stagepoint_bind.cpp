/*
	MBAPI 4.
*/

#include <mbapi_4/h/stagepoint_bind.hpp>
#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>
#include <mbapi_4/impl/h/stagepoint_bind_impl.hpp>

namespace mbapi_4
{

//
// event_subscriber_t
//

event_subscriber_t::event_subscriber_t(
	const so_5::rt::state_t & state )
	:
		m_state( state )
{
}

event_subscriber_t::~event_subscriber_t()
{
}


//
// message_integrator_t
//

message_integrator_t::~message_integrator_t()
{
}

//
// stagepoint_bind_t
//

stagepoint_bind_t::stagepoint_bind_t(
	std::unique_ptr< impl::stagepoint_bind_impl_t > impl )
	:
		m_impl( std::move( impl ) )
{
}

stagepoint_bind_t::~stagepoint_bind_t()
{
}

const stagepoint_t &
stagepoint_bind_t::stagepoint() const
{
	return m_impl->stagepoint();
}

so_5::ret_code_t
stagepoint_bind_t::subscribe_event_impl(
	const oess_id_wrapper_t & oess_id,
	event_subscriber_t & event_subscriber,
	std::unique_ptr< message_integrator_t > message_integrator,
	so_5::throwing_strategy_t throwing_strategy )
{
	return m_impl->subscribe_event_impl(
		oess_id,
		event_subscriber,
		std::move( message_integrator ),
		throwing_strategy );
}

so_5::ret_code_t
stagepoint_bind_t::unsubscribe_event_impl(
	const oess_id_wrapper_t & oess_id,
	event_subscriber_t & event_subscriber,
	std::unique_ptr< message_integrator_t > message_integrator,
	so_5::throwing_strategy_t throwing_strategy )
{
	return m_impl->unsubscribe_event_impl(
		oess_id,
		event_subscriber,
		std::move( message_integrator ),
		throwing_strategy );
}

so_5::ret_code_t
stagepoint_bind_t::bin_msg_subscribe_event_impl(
	event_subscriber_t & event_subscriber,
	so_5::throwing_strategy_t throwing_strategy )
{
	return m_impl->bin_msg_subscribe_event_impl(
		event_subscriber,
		throwing_strategy );
}

so_5::ret_code_t
stagepoint_bind_t::bin_msg_unsubscribe_event_impl(
	event_subscriber_t & event_subscriber,
	so_5::throwing_strategy_t throwing_strategy )
{
	return m_impl->bin_msg_unsubscribe_event_impl(
		event_subscriber,
		throwing_strategy );
}

} /* namespace mbapi_4 */
