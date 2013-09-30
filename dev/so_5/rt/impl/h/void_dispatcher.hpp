/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of the special empty implementation 
		of the dispatcher interface.
*/

#if !defined( _SO_5__RT__IMPL__VOID_DISPATCHER_HPP_ )
#define _SO_5__RT__IMPL__VOID_DISPATCHER_HPP_

#include <so_5/rt/h/disp.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// void_dispatcher_t
//

//! A special empty implementation of the dispatcher interface.
/*!
 * A reference to the instance of this class is passed to all agents
 * before they will be bound to actual dispatchers.
 */
class void_dispatcher_t
	:
		public dispatcher_t
{
	public:
		void_dispatcher_t();
		virtual ~void_dispatcher_t();

		virtual void
		start();

		virtual void
		shutdown();

		virtual void
		wait();

		virtual void
		put_event_execution_request(
			const agent_ref_t & agent_ref,
			unsigned int request_count );
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
