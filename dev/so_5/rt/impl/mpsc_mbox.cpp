/*
	SObjectizer 5.
*/

#include <algorithm>

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
// mpsc_mbox_t
//

mpsc_mbox_t::mpsc_mbox_t(
	mbox_id_t id,
	agent_t * single_consumer )
	:	m_id( id )
	,	m_single_consumer( single_consumer )
{
}

mpsc_mbox_t::~mpsc_mbox_t()
{
}

void
mpsc_mbox_t::subscribe_event_handler(
	const std::type_index & type_wrapper,
	agent_t * subscriber )
{
	if( subscriber != m_single_consumer )
		SO_5_THROW_EXCEPTION(
				rc_illegal_subscriber_for_mpsc_mbox,
				"the only one consumer can create subscription to mpsc_mbox" );
}

void
mpsc_mbox_t::unsubscribe_event_handlers(
	const std::type_index & type_wrapper,
	agent_t * subscriber )
{
	// Nothing to do.
}

void
mpsc_mbox_t::deliver_message(
	const std::type_index & msg_type,
	const message_ref_t & message_ref ) const
{
	agent_t::call_push_event(
			*m_single_consumer, m_id, msg_type, message_ref );
}

void
mpsc_mbox_t::deliver_service_request(
	const std::type_index & msg_type,
	const message_ref_t & svc_request_ref ) const
{
	agent_t::call_push_service_request(
			*m_single_consumer, m_id, msg_type, svc_request_ref );
}

const std::string g_mbox_empty_name;

const std::string &
mpsc_mbox_t::query_name() const
{
	return g_mbox_empty_name;
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

