/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.4
 * \brief A public interface for control SObjectizer monitoring options.
 */

#pragma once

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace rt
{

namespace stats
{

/*!
 * \since v.5.5.4
 * \brief A public interface for control SObjectizer monitoring options.
 */
class SO_5_TYPE controller_t
	{
	protected :
		~controller_t();

	public :
		//! Get the mbox for receiving monitoring information.
		virtual const mbox_t &
		mbox() const = 0;

		//! Turn the monitoring on.
		virtual void
		turn_on() = 0;

		//! Turn the monitoring off.
		virtual void
		turn_off() = 0;
	};

} /* namespace stats */

} /* namespace rt */

} /* namespace so_5 */


