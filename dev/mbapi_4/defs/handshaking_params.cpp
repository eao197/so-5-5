/*
	MBAPI 4.
*/

/*!
	\file
	\brief Параметры процедуры handshake между mbapi-агентами.
*/

#include <mbapi_4/h/declspec.hpp>
#include <mbapi_4/defs/h/handshaking_params.hpp>

namespace mbapi_4
{

//
// handshaking_params_t
//

handshaking_params_t::handshaking_params_t(
	bool is_compression_enabled )
	:
		m_is_compression_enabled( is_compression_enabled )
{
}

bool
handshaking_params_t::is_compression_enabled() const
{
	return m_is_compression_enabled;
}

handshaking_params_t &
handshaking_params_t::enable_compression()
{
	m_is_compression_enabled = true;

	return *this;
}

handshaking_params_t &
handshaking_params_t::disable_compression()
{
	m_is_compression_enabled = false;

	return *this;
}

} /* namespace mbapi_4 */
