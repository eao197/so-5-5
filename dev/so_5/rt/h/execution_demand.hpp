/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.4.0
	\brief Event-related stuff.
*/

#if !defined( _SO_5__RT__EXECUTION_DEMAND_HPP_ )
#define _SO_5__RT__EXECUTION_DEMAND_HPP_

#include <so_5/rt/h/message.hpp>

namespace so_5
{

namespace rt
{

class agent_t;

//
// event_handler_method_t
//
/*!
 * \since v.5.3.0
 * \brief Type of event handler method.
 */
typedef std::function< void(invocation_type_t, message_ref_t &) >
		event_handler_method_t;

struct execution_demand_t;

//
// demand_handler_pfn_t
//
/*!
 * \since v.5.2.0
 * \brief Demand handler prototype.
 */
typedef void (*demand_handler_pfn_t)( execution_demand_t & );

//
// execution_demand_t
//
/*!
 * \since v.5.4.0
 * \brief A description of event execution demand.
 */
struct execution_demand_t
{
	//! Receiver of demand.
	agent_t * m_receiver;
	//! ID of mbox.
	mbox_id_t m_mbox_id;
	//! Type of the message.
	std::type_index m_msg_type;
	//! Event incident.
	message_ref_t m_message_ref;
	//! Demand handler.
	demand_handler_pfn_t m_demand_handler;

	//! Default constructor.
	execution_demand_t()
		:	m_receiver( nullptr )
		,	m_mbox_id( 0 )
		,	m_msg_type( typeid(void) )
		,	m_demand_handler( nullptr )
		{}

	execution_demand_t(
		agent_t * receiver,
		mbox_id_t mbox_id,
		std::type_index msg_type,
		message_ref_t message_ref,
		demand_handler_pfn_t demand_handler )
		:	m_receiver( receiver )
		,	m_mbox_id( mbox_id )
		,	m_msg_type( msg_type )
		,	m_message_ref( std::move( message_ref ) )
		,	m_demand_handler( demand_handler )
		{}
};

} /* namespace rt */

} /* namespace so_5 */

#endif

