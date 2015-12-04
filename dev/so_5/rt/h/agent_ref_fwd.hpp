/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.2.0
	\brief A forward declaration of agent_ref_t.
*/

#if !defined( _SO_5__RT__AGENT_REF_FWD_HPP_ )
#define _SO_5__RT__AGENT_REF_FWD_HPP_

#include <so_5/h/atomic_refcounted.hpp>

#include <so_5/rt/h/fwd.hpp>

namespace so_5
{

//
// agent_ref_t
//
//! A smart reference to an agent.
/*!
 * \note Defined as typedef since v.5.2.0
 */
using agent_ref_t = intrusive_ptr_t< agent_t >;

namespace rt
{

/*!
 * \deprecated Will be removed in v.5.6.0. Use so_5::agent_ref_t
 * instead.
 */
using agent_ref_t = so_5::agent_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

