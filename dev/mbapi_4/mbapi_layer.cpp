/*
	MBAPI 4.
*/

#include <mbapi_4/h/mbapi_layer.hpp>
#include <mbapi_4/h/message.hpp>

#include <mbapi_4/impl/h/types.hpp>
#include <mbapi_4/impl/h/mbapi_layer_impl.hpp>

namespace mbapi_4
{

//
// mbapi_layer_t
//

mbapi_layer_t::mbapi_layer_t()
{
}

mbapi_layer_t::~mbapi_layer_t()
{
}

so_5::ret_code_t
mbapi_layer_t::start()
{
	m_impl.reset(
		new impl::mbapi_layer_impl_t( so_environment(), *this ) );

	return 0;
}

void
mbapi_layer_t::shutdown()
{
	m_impl.reset();
}

void
mbapi_layer_t::wait()
{
}

impl::mbapi_layer_impl_t &
mbapi_layer_t::impl()
{
	return *m_impl;
}

const impl::mbapi_layer_impl_t &
mbapi_layer_t::impl() const
{
	return *m_impl;
}

void
mbapi_layer_t::add_repository(
	const oess_2::stdsn::types_repository_t & types_repository )
{
	impl().add_repository( types_repository );
}

void
mbapi_layer_t::remove_repository(
	const oess_2::stdsn::types_repository_t & types_repository )
{
	impl().remove_repository( types_repository );
}

stagepoint_bind_unique_ptr_t
mbapi_layer_t::create_stagepoint_bind(
	const stagepoint_t & stagepoint,
	so_5::rt::agent_t & agent )
{
	stagepoint_bind_unique_ptr_t res;
	impl::stagepoint_bind_impl_unique_ptr_t
		bind_impl = impl().create_bind( stagepoint, agent );

	if( 0 != bind_impl.get() )
	{
		res.reset( new stagepoint_bind_t( std::move( bind_impl ) ) );
	}

	return res;
}

endpoint_bind_unique_ptr_t
mbapi_layer_t::create_endpoint_bind(
	const endpoint_t & endpoint,
	so_5::rt::agent_t & agent )
{
	return create_endpoint_bind( endpoint_stage_chain_t( endpoint ), agent );
}

endpoint_bind_unique_ptr_t
mbapi_layer_t::create_endpoint_bind(
	const endpoint_stage_chain_t & endpoint_stage_chain,
	so_5::rt::agent_t & agent )
{
	endpoint_bind_unique_ptr_t res;
	impl::stagepoint_bind_impl_unique_ptr_t
		bind_impl = impl().create_bind(
			endpoint_stage_chain,
			agent );

	if( 0 != bind_impl.get() )
	{
		res.reset( new endpoint_bind_t( std::move( bind_impl ) ) );
	}

	return res;
}

endpoint_list_t
mbapi_layer_t::query_endpoint_list() const
{
	return impl().query_endpoint_list();
}

void
mbapi_layer_t::send(
	const endpoint_t & from,
	const endpoint_t & to,
	const stagepoint_t & current_stage,
	const oess_id_wrapper_t & oess_id,
	std::unique_ptr< oess_2::stdsn::serializable_t > msg )
{
	m_impl->transmit_message(
		comm::local_transmit_info_unique_ptr_t(
			new comm::local_transmit_info_t(
				from,
				to,
				current_stage,
				oess_id,
				std::move( msg ) ) ) );
}

void
mbapi_layer_t::send(
	const endpoint_t & from,
	const endpoint_t & to,
	const oess_id_wrapper_t & oess_id,
	std::unique_ptr< oess_2::stdsn::serializable_t > msg )
{
	send(
		from,
		to,
		stagepoint_t( from.name(), from.name() ),
		oess_id,
		std::move( msg ) );
}

} /* namespace mbapi_4 */
