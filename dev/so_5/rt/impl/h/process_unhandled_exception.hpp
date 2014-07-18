/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \file
 * \brief Helpers for handling unhandled exceptions from agent's event handlers.
 */

#pragma once

#include <so_5/rt/h/agent.hpp>

namespace so_5 {

namespace rt {

namespace impl {

/*!
 * \since v.5.4.0
 * \brief Processor of unhandled exception from agent's event handler.
 */
void
process_unhandled_exception(
	//! Raised and caught exception.
	const std::exception & ex,
	//! Agent who is the producer of the exception.
	so_5::rt::agent_t & a_exception_producer );

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

