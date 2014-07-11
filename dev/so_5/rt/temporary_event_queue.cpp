/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.4.0
	\brief An implementation of event queue for temporary storing of events.
*/

#include <so_5/rt/h/temporary_event_queue.hpp>

#include <queue>
#include <cstdlib>

#include <so_5/h/log_err.hpp>

namespace so_5
{

namespace rt
{

//
// temporary_event_queue_t::temporary_queue_t
//
class temporary_event_queue_t::temporary_queue_t
	{
	public :
		struct item_t
			{
				agent_t * m_receiver;
				event_caller_block_ref_t m_event_caller_block;
				message_ref_t m_message_ref;
				demand_handler_pfn_t m_demand_handler;

				item_t(
					agent_t * receiver,
					event_caller_block_ref_t event_caller_block,
					message_ref_t message_ref,
					demand_handler_pfn_t demand_handler )
					:	m_receiver( receiver )
					,	m_event_caller_block( std::move( event_caller_block ) )
					,	m_message_ref( std::move( message_ref ) )
					,	m_demand_handler( demand_handler )
					{}
			};

		std::queue< item_t > m_queue;
	};

temporary_event_queue_t::temporary_event_queue_t(
	std::mutex & mutex )
	:	m_mutex( mutex )
	,	m_actual_queue( nullptr )
	{}

temporary_event_queue_t::~temporary_event_queue_t()
	{}

void
temporary_event_queue_t::push(
	agent_t * receiver,
	const event_caller_block_ref_t & event_caller_block,
	const message_ref_t & message_ref,
	demand_handler_pfn_t demand_handler )
	{
		std::lock_guard< std::mutex > lock( m_mutex );

		if( m_actual_queue )
			// Demand must go directly to the actual event queue.
			m_actual_queue->push(
					receiver,
					event_caller_block,
					message_ref,
					demand_handler );
		else
			{
				if( !m_tmp_queue )
					m_tmp_queue.reset( new temporary_queue_t() );

				m_tmp_queue->m_queue.emplace(
						receiver,
						event_caller_block,
						message_ref,
						demand_handler );
			}
	}

void
temporary_event_queue_t::switch_to_actual_queue(
	event_queue_t & actual_queue,
	agent_t * agent,
	demand_handler_pfn_t start_demand_handler )
	{
		std::lock_guard< std::mutex > lock( m_mutex );

		// All exceptions below would lead to unpredictable
		// application state. Because of that an exception would
		// lead to std::abort().
		try
			{
				actual_queue.push(
						agent,
						event_caller_block_ref_t(),
						message_ref_t(),
						start_demand_handler );

				if( m_tmp_queue )
					while( !m_tmp_queue->m_queue.empty() )
						{
							const auto & i = m_tmp_queue->m_queue.front();

							actual_queue.push(
									i.m_receiver,
									i.m_event_caller_block,
									i.m_message_ref,
									i.m_demand_handler );

							m_tmp_queue->m_queue.pop();
						}

				m_actual_queue = &actual_queue;
			}
		catch( const std::exception & x )
			{
				ACE_ERROR(
						(LM_EMERGENCY,
						 SO_5_LOG_FMT(
							 	"Exception during transferring events from "
								"temporary to the actual event queue. "
								"Work cannot be continued. "
								"Exception: %s" ),
								x.what() ) );

				std::abort();
			}
	}

} /* namespace rt */

} /* namespace so_5 */

