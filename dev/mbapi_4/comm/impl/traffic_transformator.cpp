/*
	MBAPI 4
*/

#include <array>

#include <zlib.h>

#include <oess_2/io/h/bin_data_size.hpp>
#include <oess_2/io/h/fixed_mem_buf.hpp>
#include <oess_2/io/h/mem_buf.hpp>

#include <so_5/h/log_err.hpp>
#include <so_5/h/exception.hpp>
#include <mbapi_4/defs/h/ret_code.hpp>

#include <mbapi_4/comm/impl/h/traffic_transformator.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

//
// traffic_transformator_iface_t
//

traffic_transformator_iface_t::~traffic_transformator_iface_t()
{
}


//
// transparent_traffic_transformator_t
//

//! Прозрачный трансформатор, не преобразовывает трафик.
class transparent_traffic_transformator_t
	:
		public traffic_transformator_iface_t
{
	public:
		virtual ~transparent_traffic_transformator_t()
		{}

		virtual oess_2::io::ibinstream_t &
		transform_input( oess_2::io::ibinstream_t & source, size_t )
		{
			return source;
		}

		virtual oess_2::io::obinstream_t &
		start_output(
			oess_2::io::obinstream_t & target )
		{
			return target;
		}

		virtual void
		finish_output()
		{
		}

		virtual const mbapi_4::proto::package_header_t::magic_number_t &
		quiery_magic_value() const
		{
			return transparent_traffic_transformator_t::magic_value();
		}

		static const mbapi_4::proto::package_header_t::magic_number_t &
		magic_value()
		{
			static const mbapi_4::proto::package_header_t::magic_number_t
				transparent_pkg_mn( 0x736f7035 );
			return transparent_pkg_mn;
		}
};

//
// create_transparent_traffic_transformator
//

traffic_transformator_unique_ptr_t
create_transparent_traffic_transformator()
{
	return traffic_transformator_unique_ptr_t(
		new transparent_traffic_transformator_t );
}

//! Трансформатор трафика zlib.
namespace zlib_transformator
{

//! Размер поля с длиной распакованного представления.
const size_t source_length_field_size =
	oess_2::io::bin_data_size_t< oess_2::uint_t >::image_size;

//! Размер поля со значением контрольной суммы.
const size_t crc32_field_size =
	oess_2::io::bin_data_size_t< oess_2::uint_t >::image_size;

//! Стандартный размер буффера при прияеме данных для упаковки.
const size_t g_default_buffer_size = 64*1024;

//
// transparent_traffic_transformator_t
//

class zlib_traffic_transformator_t
	:
		public traffic_transformator_iface_t
{
	public:
		zlib_traffic_transformator_t()
			:
				m_input_buffer( g_default_buffer_size ),
				m_output_buffer( g_default_buffer_size ),
				m_current_target( 0 )
		{}

		virtual ~zlib_traffic_transformator_t()
		{}

		virtual oess_2::io::ibinstream_t &
		transform_input(
			oess_2::io::ibinstream_t & source,
			size_t stream_size );

		virtual oess_2::io::obinstream_t &
		start_output(
			oess_2::io::obinstream_t & target )
		{
			m_current_target = &target;

			// Оставляем место для записи заголовка.
			m_output_buffer.change_size( mbapi_4::proto::package_header_t::IMAGE_SIZE );
			m_output_buffer.set_pos( mbapi_4::proto::package_header_t::IMAGE_SIZE );

			return m_output_buffer;
		}

		virtual void
		finish_output();

		virtual const mbapi_4::proto::package_header_t::magic_number_t &
		quiery_magic_value() const
		{
			static const mbapi_4::proto::package_header_t::magic_number_t
				zlib_pkg_mn( 0x7A6C6962 );
			return zlib_pkg_mn;
		}

	protected:
		//! Буфер для хранения распакованных данных.
		oess_2::io::mem_buf_t m_input_buffer;

		//! Буфер для приема данных, которые затем будут упакованы.
		oess_2::io::mem_buf_t m_output_buffer;

		//! Указатель на поток в который сбрасывать упакованные данные.
		oess_2::io::obinstream_t * m_current_target;
};

void
copy_data(
	oess_2::io::ibinstream_t & from,
	size_t stream_size,
	oess_2::io::mem_buf_t & to )
{
	std::array< char, 64 > buffer;

	for(; buffer.size() <= stream_size; stream_size -= buffer.size() )
	{
		from.read( buffer.data(), buffer.size() );
		to.write( buffer.data(), buffer.size() );
	}

	if( 0 != stream_size )
	{
		from.read( buffer.data(), stream_size );
		to.write( buffer.data(), stream_size );
	}
}

oess_2::io::ibinstream_t &
zlib_traffic_transformator_t::transform_input(
	oess_2::io::ibinstream_t & source,
	size_t stream_size )
{
	oess_2::io::mem_buf_t in( stream_size );
	copy_data( source, stream_size, in );
	const size_t package_size = in.size();

	// Проверяем контрольную сумму.
	uLong crc = crc32( 0L, Z_NULL, 0 );
	crc = crc32(
		crc,
		reinterpret_cast< const Bytef * >( in.ptr() ),
		package_size - crc32_field_size );

	in.set_pos( package_size - crc32_field_size );

	oess_2::uint_t crc_from_package;
	in >> crc_from_package;

	if( crc != crc_from_package )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT(
				"CRC32 mismatch; calculated crc: %X"
				", crc_from_package: %X" ), crc, crc_from_package ) );

		throw so_5::exception_t(
			"unpack failed: CRC32 mismatch",
			rc_zlib_crc_mismatch );
	}

	in.set_pos( 0 );
	// Распаковываем данные.
	oess_2::uint_t source_length;
	in >> source_length;

	m_input_buffer.change_capacity( source_length * sizeof( Bytef ), false );
	m_input_buffer.change_size( source_length );

	uLong dest_length = source_length;
	const int z_result = uncompress(
		reinterpret_cast< Bytef * >(
			const_cast< oess_2::char_t * >( m_input_buffer.ptr() ) ),
		&dest_length,
		reinterpret_cast< const Bytef * >( in.current_ptr() ),
		package_size - source_length_field_size - crc32_field_size );

	if( Z_OK != z_result )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT(
				"zlib uncompress() failure; z_result: %d" ), z_result ) );

		throw so_5::exception_t(
			"zlib uncompress() failure",
			rc_zlib_uncompress_failure );
	}

	m_input_buffer.set_pos( 0 );

	// Внутрь должен быть завернут обычный sop4 пакет.
	if( mbapi_4::proto::package_header_t::IMAGE_SIZE > m_input_buffer.size() )
	{
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT(
				"uncompressed package has no space for header" ) ) );

		throw so_5::exception_t(
			"uncompressed package has no space for header",
			rc_zlib_uncompressed_pkg_has_no_header );
	}

	mbapi_4::proto::package_header_t header(
		transparent_traffic_transformator_t::magic_value() );

	m_input_buffer >> header;

	if( !header.is_valid() )
	{
		throw so_5::exception_t(
			"invalid SOP package header: " +
				header.debug_string_representation(),
			rc_proto_invalid_header );
	}

	// После зачитки заголовка, в m_input_buffer
	// осталось только содержание пакета без заголовка.

	return m_input_buffer;
}


void
zlib_traffic_transformator_t::finish_output()
{
	if( 0 == m_current_target )
		return;

	// Записываем обычный заголовок.
	const mbapi_4::proto::package_header_t package_header(
		transparent_traffic_transformator_t::magic_value(),
		m_output_buffer.size() - mbapi_4::proto::package_header_t::IMAGE_SIZE );

	m_output_buffer.set_pos( 0 );
	package_header.write( m_output_buffer );

	const size_t predicted_compressed_data_size =
		compressBound( m_output_buffer.size() );

	oess_2::io::mem_buf_t to(
		(source_length_field_size + predicted_compressed_data_size) *
			sizeof( Bytef ) );

	const oess_2::uint_t output_buffer_size = m_output_buffer.size();
	to << output_buffer_size;

	uLong dest_length = predicted_compressed_data_size;
	const int z_result = compress(
			reinterpret_cast< Bytef * >(
				const_cast< oess_2::char_t * >( to.current_ptr() ) ),
			&dest_length,
			reinterpret_cast< const Bytef * >( m_output_buffer.ptr() ),
			output_buffer_size );

	if( Z_OK != z_result )
	{
		m_current_target = 0;
		ACE_ERROR(( LM_ERROR,
			SO_5_LOG_FMT(
				"zlib compress() failure; z_result: %d" ), z_result ) );

		throw so_5::exception_t(
			"zlib uncompress() failure",
			rc_zlib_compress_failure );
	}

	const oess_2::uint_t compressed_size(
		static_cast< oess_2::uint_t >( dest_length ) );

	// Подсчитываем CRC32.
	uLong crc = crc32( 0L, Z_NULL, 0 );
	crc = crc32(
			crc,
			reinterpret_cast< const Bytef * >( to.ptr() ),
			to.size() + compressed_size );
	oess_2::uint_t stored_crc( static_cast< oess_2::uint_t >( crc ) );

	m_current_target->write( to.ptr(), to.size()  + compressed_size );
	*m_current_target << stored_crc;

	m_current_target = 0;
	if( g_default_buffer_size != m_output_buffer.capacity() )
		m_output_buffer.change_capacity( g_default_buffer_size, false );
}

} /* namespace zlib_transformator */

traffic_transformator_unique_ptr_t
create_zlib_traffic_transformator()
{
	return traffic_transformator_unique_ptr_t(
		new zlib_transformator::zlib_traffic_transformator_t );
}

} /* namespace impl */

} /* namespace comm */

} /* namespace mbapi_4 */
