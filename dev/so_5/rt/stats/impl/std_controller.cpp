/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.4
 * \brief A standard implementation of controller for run-time monitoring.
 */

#include <so_5/rt/stats/impl/h/std_controller.hpp>

namespace so_5
{

namespace rt
{

namespace stats
{

namespace impl
{

//
// std_controller_t
//

std_controller_t::std_controller_t(
	mbox_t mbox )
	:	m_mbox( std::move( mbox ) )
	{}

std_controller_t::~std_controller_t()
	{}

const mbox_t &
std_controller_t::mbox() const
	{
		return m_mbox;
	}

void
std_controller_t::turn_on()
	{
		std::lock_guard< std::mutex > lock{ m_start_stop_lock };

		if( !m_distribution_thread )
			{
				// Distribution thread must be started.
				m_shutdown_initiated = false;
				m_distribution_thread.reset(
						new std::thread( [this] { body(); } ) );
			}
	}

void
std_controller_t::turn_off()
	{
		std::lock_guard< std::mutex > lock{ m_start_stop_lock };

		if( m_distribution_thread )
			{
				{
					// Send shutdown signal to work thread.
					std::lock_guard< std::mutex > cond_lock{ m_data_lock };
					m_shutdown_initiated = true;

					m_wake_up_cond.notify_one();
				}

				// Wait for work thread termination.
				m_distribution_thread->join();

				// Pointer to work thread must be dropped.
				// This allows to start new working thread.
				m_distribution_thread.reset();
			}
	}

void
std_controller_t::body()
	{
//FIXME: it is just of skeleton for the future implementation
//of data-distribution thread body.
		while( true )
			{
				std::unique_lock< std::mutex > lock{ m_data_lock };

				if( m_shutdown_initiated )
					return;

				m_wake_up_cond.wait_for(
						lock,
						std::chrono::milliseconds( 100 ) );
			}
	}

} /* namespace impl */

} /* namespace stats */

} /* namespace rt */

} /* namespace so_5 */

