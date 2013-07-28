/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Параметры транспортного канала.
*/

#if !defined( _SO_5_TRANSPORT__CHANNEL_PARAMS_HPP_ )
#define _SO_5_TRANSPORT__CHANNEL_PARAMS_HPP_

#include <so_5_transport/h/declspec.hpp>

#include <so_5_transport/h/scattered_block_stream.hpp>

namespace so_5_transport
{

//
// channel_params_t
//

//! Инкапсуляция различных параметров транспортного канала.
class SO_5_TRANSPORT_TYPE channel_params_t
{
	public:
		channel_params_t();
		~channel_params_t();

		//! \name Параметры входящего потока.
		//! \{
		const scattered_block_stream_params_t &
		channel_input_stream_params() const;

		channel_params_t &
		set_channel_input_stream_params(
			const scattered_block_stream_params_t & v );
		//! \}

		//! \name Параметры исходящего потока.
		//! \{
		const scattered_block_stream_params_t &
		channel_output_stream_params() const;

		channel_params_t &
		set_channel_output_stream_params(
			const scattered_block_stream_params_t & v );
		//! \}

		//! \name Период повтора проверок блокировки канала (в секундах).
		//! \{
		unsigned int
		time_checking_period() const;

		channel_params_t &
		set_time_checking_period(
			unsigned int v );
		//! \}

		//! \name Максимальное время блокировки входящих данных (в секундах).
		//! \{
		unsigned int
		max_input_block_timeout() const;

		channel_params_t &
		set_max_input_block_timeout(
			unsigned int v );
		//! \}

		//! \name Максимальное время блокировки исходящих данных (в секундах).
		//! \{
		unsigned int
		max_output_block_timeout() const;

		channel_params_t &
		set_max_output_block_timeout(
			unsigned int v );
		//! \}

	private:
		//! Парметры scattered_block_stream_t входящего канала.
		scattered_block_stream_params_t m_channel_input_stream_params;

		//! Парметры scattered_block_stream_t исходящего канала.
		scattered_block_stream_params_t m_channel_output_stream_params;


		//! \name Период повтора проверок блокировки канала (в секундах).
		/*!
			По умолчанию - 1 s.
		*/
		unsigned int m_time_checking_period;

		//! \name Максимальное время блокировки входящих данных (в секундах).
		/*!
			По умолчанию - 30 s.
		*/
		unsigned int m_max_input_block_timeout;

		//! \name Максимальное время блокировки исходящих данных (в секундах).
		/*!
			По умолчанию - 30 s.
		*/
		unsigned int m_max_output_block_timeout;
};

} /* namespace so_5_transport */

#endif
