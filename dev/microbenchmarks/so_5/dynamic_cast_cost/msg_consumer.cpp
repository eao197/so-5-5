#include "msg_consumer.hpp"

msg_t::~msg_t()
	{}

message_consumer_t::message_consumer_t()
	:	m_messages_received( 0 )
	{}

void
message_consumer_t::handle( const msg_t & msg )
	{
		m_messages_received++;
	}

