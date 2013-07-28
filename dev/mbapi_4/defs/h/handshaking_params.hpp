/*
	MBAPI 4.
*/

/*!
	\file
	\brief Параметры процедуры handshake между mbapi-агентами.
*/

#if !defined( _MBAPI_4__DEFS__HANDSHAKING_PARAMS_HPP_ )
#define _MBAPI_4__DEFS__HANDSHAKING_PARAMS_HPP_

#include <mbapi_4/defs/h/declspec.hpp>

namespace mbapi_4
{

//
// handshaking_params_t
//

/*!
	\brief Параметры процедуры handshake между канальными mbapi_4 агентами.
*/
class MBAPI_4_DEFS_TYPE handshaking_params_t
{
	public:

		/*!
			Отключает компрессию.
		*/
		handshaking_params_t(
			//! Доступна ли компрессия данных.
			/*!
				По умолчанию компрессия отключена.
			*/
			bool is_compression_enabled = false );

		/*!
			\name Методы включения/выключения компрессии.
		*/
		//! \{
		bool
		is_compression_enabled() const;

		handshaking_params_t &
		enable_compression();

		handshaking_params_t &
		disable_compression();
		//! \}

	private :
		//! Признак разрешенности/запрещенности компрессии.
		bool m_is_compression_enabled;
};

} /* namespace mbapi_4 */

#endif
