/*
	SObjectizer 5.
*/

#include <cstdlib>

#include <so_5/rt/impl/h/void_dispatcher.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

void_dispatcher_t::void_dispatcher_t()
{
}

void_dispatcher_t::~void_dispatcher_t()
{
}

void
void_dispatcher_t::start()
{
	// This method cannot be called on objects of this class.
	std::abort();
}

void
void_dispatcher_t::shutdown()
{
	// This method cannot be called on objects of this class.
	std::abort();
}

void
void_dispatcher_t::wait()
{
	// This method cannot be called on objects of this class.
	std::abort();
}

void
void_dispatcher_t::put_event_execution_request(
	const agent_ref_t &,
	unsigned int )
{
	// Do nothing.
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

