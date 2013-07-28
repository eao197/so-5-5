/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Создатель потоков канала.
*/

#if !defined( _SO_5_TRANSPORT__CHANNEL_IO_HPP_ )
#define _SO_5_TRANSPORT__CHANNEL_IO_HPP_

#include <so_5_transport/h/declspec.hpp>

#include <so_5_transport/h/scattered_block_stream.hpp>
#include <so_5_transport/h/channel_stream.hpp>
#include <so_5_transport/h/io_trx.hpp>

namespace so_5_transport
{

//
// channel_io_t
//

//! Класс для получения потоков канала.
/*!
	Класс с интерфейсом проведения транзакций чтения записи данных.
*/
class SO_5_TRANSPORT_TYPE channel_io_t
{
	public:
		channel_io_t();
		channel_io_t(
			const scattered_block_stream_ref_t & incoming,
			const scattered_block_stream_ref_t & outgoing );
		~channel_io_t();


		//! \name Интерфейс для создания потоков чтения/записи.
		//! \{

		//! Создать поток для чтения.
		input_trx_unique_ptr_t
		begin_input_trx() const;

		//! Создать поток для записи.
		output_trx_unique_ptr_t
		begin_output_trx(
			//! Время которое можно потратить на ожидание
			//! свободных блоков в случае их отсутствия или исчерпания
			//! в ходе записи.
			const ACE_Time_Value &
				free_blocks_awaiting_timeout = ACE_Time_Value::zero ) const;
		//! \}

		// //! Установить служебные потоки канала.
		// /*!
			// Обращается к методу объекта.
			// Нужен для того чтобы агент получатель сообщения
			// msg_channel_created случайно не установил свои потоки.
		// */
		// static void
		// set_streams(
			// channel_io_t & channel_io,
			// const scattered_block_stream_ref_t & in,
			// const scattered_block_stream_ref_t & out );

		// //! Установить служебный входящий поток канала.
		// static void
		// set_incoming_stream(
			// channel_io_t & channel_io,
			// const scattered_block_stream_ref_t & in );

		// //! Установить служебный исходящий поток канала.
		// static void
		// set_outgoing_stream(
			// channel_io_t & channel_io,
			// const scattered_block_stream_ref_t & out );


		//! \name Статические методы для явного доступа к блочным потокам.
		//! \{
		static inline scattered_block_stream_ref_t &
		access_input(
			channel_io_t & io )
		{
			return io.m_input;
		}

		static inline scattered_block_stream_ref_t &
		access_output(
			channel_io_t & io )
		{
			return io.m_output;
		}

		//! \}

	private:
		// //! \name Установить служебные потоки канала.
		// //! \{
		// void
		// set_incoming_stream(
			// const scattered_block_stream_ref_t & in );

		// void
		// set_outgoing_stream(
			// const scattered_block_stream_ref_t & out );
		// //! \}

		//! Блочный поток для чтения.
		scattered_block_stream_ref_t m_input;

		//! Блочный поток для записи.
		scattered_block_stream_ref_t m_output;
};


} /* namespace so_5_transport */

#endif
