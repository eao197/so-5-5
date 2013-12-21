/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/rt/impl/h/named_local_mbox.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// named_local_mbox_t
//

named_local_mbox_t::named_local_mbox_t(
	const std::string & name,
	const mbox_ref_t & mbox,
	impl::mbox_core_t & mbox_core )
	:
		m_name( name ),
		m_mbox_core( &mbox_core ),
		m_mbox( mbox )
{
}

named_local_mbox_t::~named_local_mbox_t()
{
	m_mbox_core->destroy_mbox( m_name );
}

void
named_local_mbox_t::subscribe_event_handler(
	const std::type_index & type_index,
	agent_t * subscriber,
	const event_caller_block_t * event_caller )
{
	m_mbox->subscribe_event_handler(
		type_index,
		subscriber,
		event_caller );
}

void
named_local_mbox_t::unsubscribe_event_handlers(
	const std::type_index & type_index,
	agent_t * subscriber )
{
	return m_mbox->unsubscribe_event_handlers(
		type_index,
		subscriber );
}

void
named_local_mbox_t::deliver_message(
	const std::type_index & type_index,
	const message_ref_t & message_ref ) const
{
	m_mbox->deliver_message(
		type_index,
		message_ref );
}

const std::string &
named_local_mbox_t::query_name() const
{
	return m_name;
}

const mbox_t *
named_local_mbox_t::cmp_ordinal() const
{
	return m_mbox->cmp_ordinal();
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
