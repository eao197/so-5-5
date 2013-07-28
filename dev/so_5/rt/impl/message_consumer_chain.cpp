/*
	SObjectizer 5.
*/

#include <so_5/rt/impl/h/message_consumer_chain.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// message_consumer_chain_t
//

message_consumer_chain_t::message_consumer_chain_t()
{
}

message_consumer_chain_t::~message_consumer_chain_t()
{
}

void
message_consumer_chain_t::push( 
	message_consumer_link_ref_t
		message_consumer_link )
{
	if( is_empty() )
	{
		m_front_link = message_consumer_link;
		m_back_link = message_consumer_link;
	}
	else
	{
		message_consumer_link->set_left( m_back_link );
		m_back_link->set_right( message_consumer_link );
		m_back_link = message_consumer_link;
	}
}


void
message_consumer_chain_t::pop_front()
{
	if( m_front_link.get() == m_back_link.get() )
	{
		m_front_link.reset();
		m_back_link.reset();
	}
	else
	{
		m_front_link = m_front_link->query_right();
		
		m_front_link->set_left( 
			message_consumer_link_ref_t() );
	}
}

void
message_consumer_chain_t::pop_back()
{
	if( m_front_link.get() == m_back_link.get() )
	{
		m_front_link.reset();
		m_back_link.reset();
	}
	else
	{
		m_back_link = m_back_link->query_left();
		m_back_link->set_right( 
			message_consumer_link_ref_t() );
	}
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
