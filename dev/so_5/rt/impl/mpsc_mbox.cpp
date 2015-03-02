/*
	SObjectizer 5.
*/

#include <algorithm>
#include <sstream>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/impl/h/mpsc_mbox.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// limitless_mpsc_mbox_t
//

limitless_mpsc_mbox_t::limitless_mpsc_mbox_t(
	mbox_id_t id,
	agent_t * single_consumer,
	event_queue_proxy_ref_t event_queue )
	:	m_id( id )
	,	m_single_consumer( single_consumer )
	,	m_event_queue( std::move( event_queue ) )
{
}

limitless_mpsc_mbox_t::~limitless_mpsc_mbox_t()
{
}

void
limitless_mpsc_mbox_t::subscribe_event_handler(
	const std::type_index & /*type_wrapper*/,
	const so_5::rt::message_limit::control_block_t * limit,
	agent_t * subscriber )
{
	if( subscriber != m_single_consumer )
		SO_5_THROW_EXCEPTION(
				rc_illegal_subscriber_for_mpsc_mbox,
				"the only one consumer can create subscription to mpsc_mbox" );
}

void
limitless_mpsc_mbox_t::unsubscribe_event_handlers(
	const std::type_index & /*type_wrapper*/,
	agent_t * /*subscriber*/ )
{
	// Nothing to do.
}

void
limitless_mpsc_mbox_t::deliver_message(
	const std::type_index & msg_type,
	const message_ref_t & message_ref ) const
{
	m_event_queue->push(
			execution_demand_t(
					m_single_consumer,
					m_id,
					msg_type,
					message_ref,
					&agent_t::demand_handler_on_message ) );
}

void
limitless_mpsc_mbox_t::deliver_service_request(
	const std::type_index & msg_type,
	const message_ref_t & svc_request_ref ) const
{
	m_event_queue->push(
			execution_demand_t(
					m_single_consumer,
					m_id,
					msg_type,
					svc_request_ref,
					&agent_t::service_request_handler_on_message ) );
}

std::string
limitless_mpsc_mbox_t::query_name() const
{
	std::ostringstream s;
	s << "<mbox:type=MPSC:id="
			<< m_id << ":consumer=" << m_single_consumer
			<< ">";

	return s.str();
}

//
// limitful_mpsc_mbox_t
//

limitful_mpsc_mbox_t::limitful_mpsc_mbox_t(
	mbox_id_t id,
	agent_t * single_consumer,
	const so_5::rt::message_limit::impl::info_storage_t & limits_storage,
	event_queue_proxy_ref_t event_queue )
	:	limitless_mpsc_mbox_t( id, single_consumer, std::move( event_queue ) )
	,	m_limits( limits_storage )
{
}

limitful_mpsc_mbox_t::~limitful_mpsc_mbox_t()
{
}

void
limitful_mpsc_mbox_t::deliver_message(
	const std::type_index & msg_type,
	const message_ref_t & message_ref ) const
{
//FIXME: do some actions with overload control.

	limitless_mpsc_mbox_t::deliver_message( msg_type, message_ref );
}

void
limitful_mpsc_mbox_t::deliver_service_request(
	const std::type_index & msg_type,
	const message_ref_t & svc_request_ref ) const
{
//FIXME: do some actions with overload control.

	limitless_mpsc_mbox_t::deliver_message( msg_type, svc_request_ref );
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

