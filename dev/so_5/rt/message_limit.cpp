/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.4
 * \brief Public part of message limit implementation.
 */

#include <so_5/rt/h/message_limit.hpp>

#include <so_5/rt/h/environment.hpp>

#include <so_5/h/error_logger.hpp>

namespace so_5
{

namespace rt
{

namespace message_limit
{

namespace impl
{

/*!
 * \since v.5.5.4
 * \brief Maximum overlimit reaction deep.
 */
const unsigned int max_overlimit_reaction_deep = 32;

SO_5_FUNC
void
abort_app_reaction( const overlimit_context_t & ctx )
	{
		SO_5_LOG_ERROR( ctx.m_receiver.so_environment().error_logger(), logger )
			logger
				<< "message limit exceeded, application will be aborted. "
				<< " msg_type: " << ctx.m_msg_type.name()
				<< ", limit: " << ctx.m_limit.m_limit
				<< ", agent: " << &(ctx.m_receiver)
				<< std::endl;
		std::abort();
	}

SO_5_FUNC
void
redirect_reaction(
	const overlimit_context_t & ctx,
	const mbox_t & to )
	{
		if( ctx.m_reaction_deep >= max_overlimit_reaction_deep )
			{
				SO_5_LOG_ERROR(
						ctx.m_receiver.so_environment().error_logger(),
						logger )
					logger
						<< "maximum message reaction deep exceeded on redirection, "
							"application will be aborted. "
						<< " msg_type: " << ctx.m_msg_type.name()
						<< ", limit: " << ctx.m_limit.m_limit
						<< ", agent: " << &(ctx.m_receiver)
						<< ", target_mbox: " << to->query_name()
						<< std::endl;
				std::abort();
			}
		else if( invocation_type_t::event == ctx.m_event_type )
			to->do_deliver_message(
					ctx.m_msg_type,
					ctx.m_message,
					ctx.m_reaction_deep + 1 );
		else if( invocation_type_t::service_request == ctx.m_event_type )
			to->do_deliver_service_request(
					ctx.m_msg_type,
					ctx.m_message,
					ctx.m_reaction_deep + 1 );
	}

} /* namespace impl */

} /* namespace message_limit */

} /* namespace rt */

} /* namespace so_5 */

