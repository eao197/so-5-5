/*
	MBAPI 4.
*/

#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>
#include <mbapi_4/h/endpoint_bind.hpp>

namespace mbapi_4
{

//
// endpoint_bind_t
//

endpoint_bind_t::endpoint_bind_t(
	std::unique_ptr< impl::stagepoint_bind_impl_t > impl )
	:
		stagepoint_bind_t( std::move( impl ) ),
		m_endpoint( m_impl->stagepoint().endpoint_name() )
{
}

endpoint_bind_t::~endpoint_bind_t()
{
}

const endpoint_t &
endpoint_bind_t::endpoint() const
{
	return m_endpoint;
}

void
endpoint_bind_t::send_impl(
	const endpoint_t & to,
	const oess_id_wrapper_t & oess_id,
	std::unique_ptr< oess_2::stdsn::serializable_t > msg )
{
	m_impl->send_impl( to, oess_id, std::move( msg ) );
}

void
endpoint_bind_t::send_binary_message(
	const endpoint_t & to,
	const oess_id_wrapper_t & oess_id,
	const std::string & binary_message )
{
	m_impl->send_binary_message_impl( to, oess_id, binary_message );
}

} /* namespace mbapi_4 */
