/*
	MBAPI 4.
*/

#include <sstream>

#include <ace/OS.h>

#include <oess_2/stdsn/h/ent_std.hpp>
#include <oess_2/io/h/subbinstream.hpp>

#include <so_5/h/log_err.hpp>
#include <so_5/h/exception.hpp>

#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/comm/impl/h/iochannel_data_processor.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

std::string
create_uid_from_addr(
	const void * ptr )
{
	std::ostringstream sout;

	sout << ptr;

	return sout.str();
}


//
// iochannel_data_processor_t
//

iochannel_data_processor_t::iochannel_data_processor_t(
	const handshaking_params_t & handshaking_params,
	const so_5::rt::mbox_ref_t & mbox,
	const so_5_transport::channel_controller_ref_t & controller,
	const so_5_transport::channel_io_t & io )
	:
		m_channel_uid( create_uid_from_addr( self_ptr() ) ),
		m_mbox( mbox ),
		m_controller( controller ),
		m_io( io ),
		m_handshaking_params( handshaking_params ),
		m_traffic_transformator( create_transparent_traffic_transformator() ),
		m_handshaked( false ),
		m_las_activity_timestamp( ACE_OS::gettimeofday() )
{
}

iochannel_data_processor_t::~iochannel_data_processor_t()
{
}

const iochannel_data_processor_t *
iochannel_data_processor_t::self_ptr() const
{
	return this;
}

void
parse(
	oess_2::io::istream_t & istream,
	proto::req_info_ptr_t & item )
{
	oess_2::stdsn::ient_std_t ient(
		istream,
		proto::req_info_t::get_repository() );

	ient >> item;
}

void
iochannel_data_processor_t::process_incoming(
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
	so_5_transport::input_trx_t & in_trx )
{
	while( proto::package_header_t::IMAGE_SIZE  <
		in_trx.stream_size() )
	{
		proto::package_header_t header(
			m_traffic_transformator->quiery_magic_value() );

		in_trx.istream() >> header;

		if( header.is_valid() )
		{
			const size_t body_size = header.size();
			if( body_size <= in_trx.stream_size() )
			{
				oess_2::io::isubbinstream_t substream(
					in_trx.istream(), body_size );

				oess_2::io::ibinstream_t & ready_data_stream =
					m_traffic_transformator->transform_input(
						substream, body_size );

				proto::req_info_ptr_t request;
				parse( ready_data_stream, request );

				if( request.get() )
					handle_request( *request.get(), mbapi_layer );
				else
					ACE_ERROR(( LM_ERROR,
						SO_5_LOG_FMT( "unable to create request" ) ));
			}
			else
			{
				// Если пакет полностью не пришел
				// то из цикла надо выходить,
				// причем транзакцию не надо комитить,
				// чтобы в следующий раз заново прочитать заголовок.
				break;
			}
		}
		else
		{
			throw so_5::exception_t(
				"invalid MBAPI package header: " +
					header.debug_string_representation(),
				rc_proto_invalid_header );
		}

		in_trx.checkpoint();
	}

	// Обновляем время последней активности.
	m_las_activity_timestamp = ACE_OS::gettimeofday();
}

void
iochannel_data_processor_t::process_outgoing(
	transmit_info_t & transmit_info )
{
	format( proto::req_info_ptr_t(
		new proto::send_msg_info_t( transmit_info ) ) );
}

void
iochannel_data_processor_t::sync_tables(
	const mbapi_4::impl::infrastructure::available_endpoint_table_t & available_endpoint_table,
	const mbapi_4::impl::infrastructure::available_stagepoint_table_t & available_stagepoint_table )
{
	if( m_handshaked )
	{
		using namespace mbapi_4::impl::infrastructure;

		std::unique_ptr< proto::sync_tables_info_t >
			request_ptr( new proto::sync_tables_info_t );

		for(
			available_endpoint_table_t::const_iterator
				it = available_endpoint_table.begin(),
				it_end = available_endpoint_table.end();
			it != it_end;
			++it )
		{
			const available_endpoint_t & aep = *(it->second);

			if( aep.m_channel_id != m_channel_uid )
			{
				proto::endpoint_info_ptr_t ep( new proto::endpoint_info_t );
				ep->m_name = it->first.name();
				ep->m_node_uid = aep.m_node_uid.str();
				ep->m_distance = aep.m_distance;
				ep->m_endpoint_stage_chain.assign(
					aep.m_endpoint_stage_chain.stages().begin(),
					aep.m_endpoint_stage_chain.stages().end() );

				request_ptr->m_endpoints.push_back( ep );
			}
		}

		for(
			available_stagepoint_table_t::const_iterator
				it = available_stagepoint_table.begin(),
				it_end = available_stagepoint_table.end();
			it != it_end;
			++it )
		{
			const available_stagepoint_t & asp = *(it->second);

			if( asp.m_channel_id != m_channel_uid )
			{
				proto::stagepoint_info_ptr_t sp( new proto::stagepoint_info_t );
				sp->m_name = it->first.name();
				sp->m_endpoint_name = it->first.endpoint_name();
				sp->m_node_uid = asp.m_node_uid.str();
				sp->m_distance = asp.m_distance;

				request_ptr->m_stagepoints.push_back( sp );
			}
		}

		format(
			proto::req_info_ptr_t(
				request_ptr.release() ) );
	}
}


const ACE_Time_Value g_five_sec( 5, 0 );
bool
iochannel_data_processor_t::is_active() const
{
	return
		( ACE_OS::gettimeofday() - m_las_activity_timestamp ) < g_five_sec;
}

void
iochannel_data_processor_t::format(
	const proto::req_info_ptr_t & item )
{
	try
	{
		so_5_transport::output_trx_unique_ptr_t out_trx =
			m_io.begin_output_trx();

		oess_2::stdsn::oent_std_t oent(
			m_traffic_transformator->start_output(
				out_trx->ostream() ) );

		oent << item;

		// Трансформируем записанный пакет.
		m_traffic_transformator->finish_output();

		proto::package_header_t package_header(
			m_traffic_transformator->quiery_magic_value(),
			out_trx->stream_size() );

		// Начинаем транзакцию для записи заголовка пакета.
		so_5_transport::output_trx_unique_ptr_t pkg_header_out_trx =
			m_io.begin_output_trx();
		package_header.write( pkg_header_out_trx->ostream() );

		// Сначала должен уйти заголовок,
		pkg_header_out_trx->commit();
		// потом само cодержимое пакета.
		out_trx->commit();
	}
	catch( std::exception & x )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT( "exception: %s" ), x.what() ));
	}
}

bool
iochannel_data_processor_t::handle_request(
	const proto::req_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	switch( request.query_req_id() )
	{
		case proto::req_id::REQ_SEND_MSG:
			handle(
				dynamic_cast< const proto::send_msg_info_t & >( request ),
				mbapi_layer );
			break;

		case proto::req_id::RESP_SEND_MSG:
			handle(
				dynamic_cast< const proto::send_msg_resp_info_t & >( request ),
				mbapi_layer );
			break;

		case proto::req_id::REQ_PING:
			handle(
				dynamic_cast< const proto::ping_info_t & >( request ),
				mbapi_layer );
			break;
		case proto::req_id::RESP_PING:
			handle(
				dynamic_cast< const proto::ping_resp_info_t & >( request ),
				mbapi_layer );
			break;

		case proto::req_id::REQ_SYNC_TABLES:
			handle(
				dynamic_cast< const proto::sync_tables_info_t & >( request ),
				mbapi_layer );
			break;
		case proto::req_id::RESP_SYNC_TABLES:
			handle(
				dynamic_cast< const proto::sync_tables_resp_info_t & >( request ),
				mbapi_layer );
			break;

		default:
			return false;
	}

	return true;
}

void
iochannel_data_processor_t::handle(
	const proto::send_msg_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	transmit_info_unique_ptr_t transmit_info(
		new transmit_info_t(
			request.m_from,
			request.m_to,
			request.m_current_stage,
			request.stage_owner_endpoint(),
			request.m_oess_id ) );

	transmit_info->m_payload.swap(
		const_cast< std::string & >( request.m_payload ) );

	mbapi_layer.transmit_message( std::move( transmit_info ) );

	format(
		proto::req_info_ptr_t(
			new proto::send_msg_resp_info_t( 0, "" )  ) );
}

void
iochannel_data_processor_t::handle(
	const proto::send_msg_resp_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	/* Ничего не делаем. */
}

void
iochannel_data_processor_t::handle(
	const proto::ping_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	format(
		proto::req_info_ptr_t(
			new proto::ping_resp_info_t ) );
}

void
iochannel_data_processor_t::handle(
	const proto::ping_resp_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	/* Ничего не делаем. */
}

void
iochannel_data_processor_t::handle(
	const proto::sync_tables_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	using namespace mbapi_4::impl::infrastructure;
	available_endpoint_table_t available_endpoint_table;

	for(
		proto::endpoint_info_table_t::const_iterator
			it = request.m_endpoints.begin(),
			it_end = request.m_endpoints.end();
			it != it_end;
			++it )
	{
		const proto::endpoint_info_t & ep = **it;

		const endpoint_t endpoint( ep.m_name );

		available_endpoint_ptr_t ep_data( new available_endpoint_t );
		ep_data->m_node_uid = mbapi_node_uid_t( ep.m_node_uid );
		ep_data->m_distance = ep.m_distance;

		ep_data->m_endpoint_stage_chain.set_endpoint( endpoint );
		ep_data->m_endpoint_stage_chain.set_stages( ep.m_endpoint_stage_chain );

		available_endpoint_table[ endpoint ] = ep_data;
	}

	available_stagepoint_table_t available_stagepoint_table;

	for(
		proto::stagepoint_info_table_t::const_iterator
			it = request.m_stagepoints.begin(),
			it_end = request.m_stagepoints.end();
			it != it_end;
			++it )
	{
		const proto::stagepoint_info_t & sp = **it;

		const stagepoint_t stagepoint( sp.m_name, sp.m_endpoint_name );

		available_stagepoint_ptr_t sp_data( new available_stagepoint_t );
		sp_data->m_node_uid = mbapi_node_uid_t( sp.m_node_uid );
		sp_data->m_distance = sp.m_distance;

		available_stagepoint_table[ stagepoint ] = sp_data;
	}

	mbapi_layer.update_channel(
		m_channel_uid,
		m_node_uid,
		m_mbox,
		available_endpoint_table,
		available_stagepoint_table );

	format(
		proto::req_info_ptr_t(
			new proto::sync_tables_resp_info_t ) );
}

void
iochannel_data_processor_t::handle(
	const proto::sync_tables_resp_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	/* Ничего не делаем. */
}

void
iochannel_data_processor_t::set_mbapi_node_uid(
	const std::string & uid )
{
	m_node_uid = mbapi_node_uid_t( uid );
}

//
// client_channel_data_processor_t
//

client_channel_data_processor_t::client_channel_data_processor_t(
	const handshaking_params_t & handshaking_params,
	const so_5::rt::mbox_ref_t & mbox,
	const so_5_transport::channel_controller_ref_t & controller,
	const so_5_transport::channel_io_t & io )
	:
		base_type_t(
			handshaking_params,
			mbox,
			controller,
			io )
{
}

client_channel_data_processor_t::~client_channel_data_processor_t()
{
}

bool
client_channel_data_processor_t::handle_request(
	const proto::req_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	if( m_handshaked )
		return base_type_t::handle_request( request, mbapi_layer );
	else if( proto::req_id::REQ_HANDSHAKE == request.query_req_id() )
	{
		handle(
			dynamic_cast< const proto::handshake_info_t & >( request ) );
		m_handshaked = true;
		return true;
	}

	return false;
}

void
client_channel_data_processor_t::handle(
	const proto::handshake_info_t & request )
{
	set_mbapi_node_uid( request.node_uid() );

	std::unique_ptr< proto::handshake_resp_info_t > handshake_resp(
		new proto::handshake_resp_info_t );

	traffic_transformator_unique_ptr_t traffic_transformator;

	// Если клиент желает компрессии в режиме zlib и нам
	// разрешено использовать компрессию, то мы должны
	// установить этот режим.
	if( m_handshaking_params.is_compression_enabled() &&
		request.compression() &&
		request.compression().is_supported(
			proto::handshake::compression_t::zlib() ) )
	{
		traffic_transformator = create_zlib_traffic_transformator();
		handshake_resp->set_compression_algorithm(
			proto::handshake::compression_t::zlib() );
	}

	format(
		proto::req_info_ptr_t( handshake_resp.release() ) );

	if( traffic_transformator.get() )
	{
		m_traffic_transformator = std::move( traffic_transformator );
	}
}

//
// client_channel_data_processor_table_t
//

bool
client_channel_data_processor_table_t::has_client_channel(
	const channel_uid_wrapper_t & channel_id ) const
{
	return m_clients.end() != m_clients.find( channel_id );
}

void
client_channel_data_processor_table_t::insert_client_channel(
	const channel_uid_wrapper_t & channel_id,
	client_channel_data_processor_unique_ptr_t client_channel_data_processor )
{
	m_clients[ channel_id ] =
		client_channel_data_processor_ptr_t(
			client_channel_data_processor.release() );
}

void
client_channel_data_processor_table_t::remove_client_channel(
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
	const channel_uid_wrapper_t & channel_id )
{
	m_clients.erase( channel_id );
	mbapi_layer.delete_channel( channel_id );
}

void
client_channel_data_processor_table_t::remove_all_client_channels(
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	for(
		processor_table_t::iterator
			it = m_clients.begin(),
			it_end = m_clients.end();
		it != it_end;
		++it )
	{
		mbapi_layer.delete_channel( it->first );
	}

	m_clients.clear();
}

void
client_channel_data_processor_table_t::process_incoming(
	const channel_uid_wrapper_t & channel_id,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer,
	so_5_transport::input_trx_t & in_trx )
{
	processor_table_t::iterator it =
		m_clients.find( channel_id );
	if( m_clients.end() != it );
		it->second->process_incoming( mbapi_layer, in_trx );
}

void
client_channel_data_processor_table_t::process_outgoing(
	transmit_info_t & transmit_info )
{
	processor_table_t::iterator it =
		m_clients.find( transmit_info.m_channel_id );

	if( m_clients.end() != it )
		it->second->process_outgoing( transmit_info );
}

void
client_channel_data_processor_table_t::close(
	const channel_uid_wrapper_t & channel_id )
{
	processor_table_t::iterator it =
		m_clients.find( channel_id );

	if( m_clients.end() != it )
		it->second->close();
}

void
client_channel_data_processor_table_t::sync_tables(
	const mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	// Если есть с кем синхронизироваться, то получаем копию
	// таблиц конечных точек и точек стадий,
	// и инициируем отправку сведений в имеющиеся каналы.
	if( !m_clients.empty() )
	{
		mbapi_4::impl::infrastructure::available_endpoint_table_t endpoints;
		mbapi_4::impl::infrastructure::available_stagepoint_table_t stagepoints;

		mbapi_layer.copy_tables_data( endpoints, stagepoints );

		for(
			processor_table_t::iterator
				it = m_clients.begin(),
				it_end = m_clients.end();
			it != it_end;
			++it )
		{
			it->second->sync_tables( endpoints, stagepoints );
		}
	}
}

void
client_channel_data_processor_table_t::check_activity()
{
	// Проверяем активность каналов.
	// Если из канала давной ничего не приходило, а должна
	// была приходить как минимум информация о синхронизации таблиц,
	// то такой канал закрываем.
	for(
		processor_table_t::iterator
			it = m_clients.begin(),
			it_end = m_clients.end();
		it != it_end;
		++it )
	{
		if( !it->second->is_active() )
		{
			ACE_ERROR(( LM_ERROR,
				SO_5_LOG_FMT( "client channel is inactive, channel_uid: %s" ),
				it->first.str().c_str() ));
			it->second->close();
		}
	}
}

//
// server_channel_data_processor_t
//

server_channel_data_processor_t::server_channel_data_processor_t(
	const handshaking_params_t & handshaking_params,
	const so_5::rt::mbox_ref_t & mbox,
	const so_5_transport::channel_controller_ref_t & controller,
	const so_5_transport::channel_io_t & io )
	:
		base_type_t(
			handshaking_params,
			mbox,
			controller,
			io )
{
}

server_channel_data_processor_t::~server_channel_data_processor_t()
{
}

void
server_channel_data_processor_t::initiate_handshake(
	const mbapi_node_uid_t & node_uid )
{
	std::unique_ptr< proto::handshake_info_t > handshake(
		new proto::handshake_info_t );

	handshake->set_node_uid( node_uid.str() );

	if( m_handshaking_params.is_compression_enabled() )
	{
		// Предпочитаем алгоритм компрессии zlib.
		handshake->set_compression(
			proto::handshake::compression_t(
				proto::handshake::compression_t::zlib(),
				// Других алгоритмов нет.
				std::set< std::string >() ) );
	}

	format(
		proto::req_info_ptr_t( handshake.release() ) );
}

bool
server_channel_data_processor_t::handle_request(
	const proto::req_info_t & request,
	mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	if( m_handshaked )
		return base_type_t::handle_request( request, mbapi_layer );
	else if( proto::req_id::RESP_HANDSHAKE == request.query_req_id() )
	{
		handle(
			dynamic_cast< const proto::handshake_resp_info_t & >( request ) );
		m_handshaked = true;
		return true;
	}

	return false;
}


void
server_channel_data_processor_t::handle(
	const proto::handshake_resp_info_t & request )
{
	set_mbapi_node_uid( request.node_uid() );

	// Если чего-то надо ставить, то ставим.
	if( request.is_compression_defined() )
	{
		// Сейчас мы поддерживаем только zlib.
		if( proto::handshake::compression_t::zlib() ==
			request.compression_algorithm() )
		{
			m_traffic_transformator = create_zlib_traffic_transformator();
		}
		else
		{
			throw so_5::exception_t(
				"unknown compression algorithm: " + request.compression_algorithm(),
				rc_unknown_compression_algorithm );
		}
	}
}

void
server_channel_data_processor_t::sync_tables(
	const mbapi_4::impl::mbapi_layer_impl_t & mbapi_layer )
{
	mbapi_4::impl::infrastructure::available_endpoint_table_t endpoints;
	mbapi_4::impl::infrastructure::available_stagepoint_table_t stagepoints;

	mbapi_layer.copy_tables_data( endpoints, stagepoints );

	base_type_t::sync_tables( endpoints, stagepoints );
}

void
server_channel_data_processor_t::check_activity()
{
	if( !is_active() )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT( "server channel is inactive, channel_uid: %" ),
			m_channel_uid.str().c_str() ));

		close();
	}
}

} /* namespace impl */

} /* namespace comm */

} /* namespace mbapi_4 */
