/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.3
 * \file
 * \brief A collector for agent tuning options.
 */

#pragma once

#include <so_5/rt/h/subscription_storage_fwd.hpp>
#include <so_5/rt/h/message_limit.hpp>

namespace so_5
{

namespace rt
{

//
// agent_tuning_options_t
//
/*!
 * \since v.5.5.3
 * \brief A collector for agent tuning options.
 */
class agent_tuning_options_t
	{
	public :
		//! Set factory for subscription storage creation.
		agent_tuning_options_t &
		subscription_storage_factory(
			subscription_storage_factory_t factory )
			{
				m_subscription_storage_factory = factory;

				return *this;
			}

		const subscription_storage_factory_t &
		query_subscription_storage_factory() const
			{
				return m_subscription_storage_factory;
			}

		//! Default subscription storage factory.
		static subscription_storage_factory_t
		default_subscription_storage_factory()
			{
				return so_5::rt::default_subscription_storage_factory();
			}

		message_limit::description_container_t
		giveout_message_limits()
			{
				return std::move( m_message_limits );
			}

		template< typename... ARGS >
		agent_tuning_options_t &
		message_limits( ARGS &&... args )
			{
				message_limit::accept_indicators(
						m_message_limits,
						std::forward< ARGS >( args )... );

				return *this;
			}

	private :
		subscription_storage_factory_t m_subscription_storage_factory =
				default_subscription_storage_factory();

		message_limit::description_container_t m_message_limits;
	};

} /* namespace rt */

} /* namespace so_5 */

