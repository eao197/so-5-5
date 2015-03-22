/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.4
 * \brief A standard implementation of controller for run-time monitoring.
 */

#pragma once

#include <so_5/rt/stats/h/controller.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <memory>

namespace so_5
{

namespace rt
{

namespace stats
{

namespace impl
{

/*!
 * \since v.5.5.4
 * \brief A standard implementation of controller for run-time monitoring.
 */
class std_controller_t : public controller_t
	{
	public :
		std_controller_t(
			mbox_t mbox );
		~std_controller_t();

		//! Get the mbox for receiving monitoring information.
		virtual const mbox_t &
		mbox() const override;

		//! Turn the monitoring on.
		virtual void
		turn_on() override;

		//! Turn the monitoring off.
		virtual void
		turn_off() override;

	private :
		//! Mbox for sending monitoring data.
		const mbox_t m_mbox;

		//! Object lock for start/stop operations.
		std::mutex m_start_stop_lock;
		//! Object lock for data-related operations.
		std::mutex m_data_lock;
		//! Condition for wake-up data-distribution thread.
		std::condition_variable m_wake_up_cond;

		//! Main data-distribution thread.
		/*!
		 * This thread is created in turn_on() and is destroyed in turn_off().
		 * It is not exists if run-time monitoring is switched off.
		 */
		std::unique_ptr< std::thread > m_distribution_thread;

		//! Shutdown signal.
		/*!
		 * It is set to true in turn_off().
		 */
		bool m_shutdown_initiated;

		//! Main body of data distribution thread.
		void
		body();
	};

} /* namespace impl */

} /* namespace stats */

} /* namespace rt */

} /* namespace so_5 */

