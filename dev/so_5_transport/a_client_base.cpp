/*
	SObjectizer 5 Transport.
*/

#include <so_5/rt/h/rt.hpp>

#include <so_5_transport/h/a_client_base.hpp>

namespace so_5_transport
{

//
// a_client_base_t
//

a_client_base_t::a_client_base_t(
	so_5::rt::so_environment_t & env,
	const so_5::rt::mbox_ref_t & notification_mbox )
	:
		base_type_t( env, notification_mbox )
{
}

a_client_base_t::~a_client_base_t()
{
}

} /* namespace so_5_transport */
