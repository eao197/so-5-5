/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.3
 * \file
 * \brief An interface of subscription storage.
 */

#pragma once

#include <ostream>

#include <so_5/h/types.hpp>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/execution_demand.hpp>
#include <so_5/rt/h/subscription_storage_fwd.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// event_handler_data_t
//
/*!
 * \since v.5.4.0
 * \brief Information about event_handler and its properties.
 */
struct event_handler_data_t
	{
		//! Method for handling event.
		event_handler_method_t m_method;
		//! Is event handler thread safe or not.
		thread_safety_t m_thread_safety;

		event_handler_data_t(
			event_handler_method_t method,
			thread_safety_t thread_safety )
			:	m_method( std::move( method ) )
			,	m_thread_safety( thread_safety )
			{}
	};

/*!
 * \since v.5.5.3
 * \brief An interface of subscription storage
 *
 * Prior to v.5.5.3 there where only one subscription_storage implementation.
 * But this implementation was not efficient for all cases.
 *
 * Sometimes an agent has very few subscriptions and efficient 
 * implementation for that case can be based on std::vector.
 * Sometimes an agent has very many subscriptions and efficient
 * implementation can require std::map or std::unordered_map.
 *
 * The purpose of this interface is hiding details of concrete
 * subscription_storage implementation.
 */
class subscription_storage_t
	{
	public :
		subscription_storage_t( agent_t * owner );
		virtual ~subscription_storage_t();

		virtual void
		create_event_subscription(
			const mbox_t & mbox,
			std::type_index msg_type,
			const state_t & target_state,
			const event_handler_method_t & method,
			thread_safety_t thread_safety ) = 0;

		virtual void
		drop_subscription(
			const mbox_t & mbox,
			const std::type_index & msg_type,
			const state_t & target_state ) = 0;

		virtual void
		drop_subscription_for_all_states(
			const mbox_t & mbox,
			const std::type_index & msg_type ) = 0;

		virtual const event_handler_data_t *
		find_handler(
			mbox_id_t mbox_id,
			const std::type_index & msg_type,
			const state_t & current_state ) const = 0;

		virtual void
		debug_dump( std::ostream & to ) const = 0;

	protected :
		agent_t *
		owner() const;

	private :
		agent_t * m_owner;
	};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

