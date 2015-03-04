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

SO_5_FUNC
void
abort_app_reaction( const overlimit_context_t & ctx )
	{
		SO_5_LOG_ERROR( ctx.m_receiver.so_environment().error_logger(), logger )
			logger
				<< "message limit exceeded, application will be aborted. "
				<< "agent: " << &(ctx.m_receiver)
				<< ", msg_type: " << ctx.m_msg_type.name()
				<< std::endl;
		std::abort();
	}

} /* namespace impl */

} /* namespace message_limit */

} /* namespace rt */

} /* namespace so_5 */

