/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A forward declaration of event_handler_caller_ref.
*/


#if !defined( _SO_5__RT__EVENT_HANDLER_CALLER_REF_FWD_HPP_ )
#define _SO_5__RT__EVENT_HANDLER_CALLER_REF_FWD_HPP_

#include <so_5/rt/h/atomic_refcounted.hpp>

namespace so_5
{

namespace rt
{

class event_handler_caller_t;

//
// event_handler_caller_ref_t
//
/*!
 * \brief A smart reference to event_handler_caller.
 * \note Defined as typedef since v.5.2.0
 */
typedef smart_atomic_reference_t< event_handler_caller_t >
	event_handler_caller_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
