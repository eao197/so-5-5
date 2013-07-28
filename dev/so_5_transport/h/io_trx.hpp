/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Класс для облегчения IO транзакций c каналом.
*/

#if !defined( _SO_5_TRANSPORT__IO_TRX_HPP_ )
#define _SO_5_TRANSPORT__IO_TRX_HPP_

#include <memory>

#include <so_5_transport/h/declspec.hpp>

#include <so_5_transport/h/channel_stream.hpp>

namespace so_5_transport
{

//
// input_trx_t
//

//! Класс транзакции чтения данных.
/*!
	При создании сразу начинает транзакцию и устанавливает
	контрольную точку до выполнения каких-либо действий.
	В ходе чтения данных можно устанавливать контрольную точку,
	которая фиксирует уже зачитанные данные.
	При уничтожении, если транзакция не была подтверждена,
	происходит откат транзакции к последней контрольной точке.
*/
class SO_5_TRANSPORT_TYPE input_trx_t
{
	public:
		input_trx_t(
			channel_input_stream_unique_ptr_t istream );

		//! Получить поток для чтения данных.
		//! \note Транзакция не должна быть завершена.
		inline oess_2::io::ibinstream_t &
		istream()
		{
			return *m_istream;
		}

		//! Зафиксировать контрольную точку и продолжить транзакцию.
		void
		checkpoint();

		//! Подтвердить транзакцию.
		void
		commit();

		//! Отменить транзакцию.
		/*!
			Откат происходит к последней контрольной точке.
		*/
		void
		rollback();

		//! Размер потока в байтах доступных для чтения.
		inline size_t
		stream_size() const
		{
			return m_istream->bytes_available();
		}

	private:
		//! Поток для чтения данных.
		channel_input_stream_unique_ptr_t m_istream;
};

//! Псевдоним unique_ptr для input_trx_t.
typedef std::unique_ptr< input_trx_t > input_trx_unique_ptr_t;

//
// input_trx_t
//

//! Класс транзакции записи данных.
/*!
	При создании сразу начинает транзакцию и устанавливает
	контрольную точку до выполнения каких-либо действий.
	В ходе записи данных можно устанавливать контрольную точку,
	которая фиксирует уже записанные данные.
	При уничтожении, если транзакция не была подтверждена,
	происходит откат транзакции к последней контрольной точке.
*/
class SO_5_TRANSPORT_TYPE output_trx_t
{
	public:
		output_trx_t(
			channel_output_stream_unique_ptr_t ostream );

		//! Получить поток для записи данных.
		//! \note Транзакция не должна быть завершена.
		inline oess_2::io::obinstream_t &
		ostream()
		{
			return *m_ostream;
		}

		//! Зафиксировать сдаланное и продолжить транзакцию.
		void
		checkpoint();

		//! Зафиксировать контрольную точку и продолжить транзакцию.
		void
		commit();

		//! Отменить транзакцию.
		void
		rollback();

		//! Размер потока в байтах, которые записаны к данному моменту.
		inline size_t
		stream_size() const
		{
			return m_ostream->bytes_written();
		}

	private:
		//! Поток для чтения данных.
		channel_output_stream_unique_ptr_t m_ostream;
};

//! Псевдоним unique_ptr для output_trx_t.
typedef std::unique_ptr< output_trx_t > output_trx_unique_ptr_t;

} /* namespace so_5_transport */

#endif
