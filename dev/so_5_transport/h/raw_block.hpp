/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Класс блока данных.
*/

#if !defined( _SO_5_TRANSPORT__RAW_BLOCK_HPP_ )
#define _SO_5_TRANSPORT__RAW_BLOCK_HPP_

#include <vector>
#include <list>

#include <oess_2/defs/h/types.hpp>

#include <so_5_transport/h/declspec.hpp>

namespace so_5_transport
{

//
// raw_block_t
//

//! Коммуникационный блок-буфер.
/*!
	Предоставляет функциональность работы с блоком памяти
	фиксированной длины.
*/
class SO_5_TRANSPORT_TYPE raw_block_t
{
		raw_block_t( const raw_block_t & o );
		void
		operator = ( const raw_block_t & o );

	public:
		//! Конструктор с возможностью задать емкость.
		/*!
			Поведение при начальной емкости 0 байт не определено.
		*/
		raw_block_t(
			/*! Емкость буфера в байтах.*/
			size_t capacity = 32 *1024 );

		~raw_block_t();

		//! Емкость блока.
		/*!
			Емкость = рамер записанных данных + рамер доступных для записи данных.
		*/
		inline size_t
		capacity() const
		{
			return m_block.capacity();
		}

		//! Указатель на начала блока памяти.
		inline const oess_2::char_t *
		ptr() const
		{
			return m_buff_ptr;
		}

		//! Указатель на начала блока памяти, содержащего буфер.
		inline oess_2::char_t *
		ptr()
		{
			return m_buff_ptr;
		}

		//! \name Сбросить позиции записи/чтения данных данного блока.
		//! \{

		//! Подготовить блок к записи.
		inline void
		reset_write_pos()
		{
			// При этом обязательно надо сбрасывать
			// позицию чтения.
			reset_read_pos();

			m_write_pos = 0;
		}

		//! Подготовить блок к чтению.
		inline void
		reset_read_pos()
		{
			m_read_pos = 0;
		}

		//! \}

		//! \name Интерфейс для записи данных.
		//! \{

		//! Текущая позиция для записи.
		inline size_t
		write_pos() const
		{
			return m_write_pos;
		}

		//! Количество байт доступных для записи.
		inline size_t
		write_size_available() const
		{
			return capacity() - write_pos();
		}

		//! Сдвинуть текущую позицию для записи.
		/*!
			Операция выполняется только если n <= write_size_available().
		*/
		inline void
		shift_write_pos( size_t n )
		{
			if( n <= write_size_available() )
				m_write_pos += n;
		}

		//! Установить текущую позицию для записи в конец.
		inline void
		total_shift_write_pos()
		{
			m_write_pos = capacity();
		}

		//! Указатель на текущую позицию для записи.
		inline oess_2::char_t *
		current_write_ptr()
		{
			return m_buff_ptr + write_pos();
		}

		//! \}

		//! \name Интерфейс для чтения данных.
		//! \{

		//! Текущая позиция для чтения.
		inline size_t
		read_pos() const
		{
			return m_read_pos;
		}

		//! Текущий размер доступных для чтения данных.
		inline size_t
		read_size_available() const
		{
			return write_pos() - read_pos();
		}

		//! Сдвинуть  текущую позицию для чтения.
		/*!
			Операция выполняется только если n <= read_size_available().
		*/
		inline void
		shift_read_pos( size_t n )
		{
			if( n <= read_size_available() )
				m_read_pos += n;
		}

		//! Установить текущую позицию для чтения в конец.
		inline void
		total_shift_read_pos()
		{
			m_read_pos = write_pos();
		}

		//! Указатель на текущую позицию для чтения.
		inline const oess_2::char_t *
		current_read_ptr() const
		{
			return m_buff_ptr + read_pos();
		}
		//! \}

	private:
		//! Хранилище буфера, заботится об удалении массива байт.
		std::vector< oess_2::char_t > m_block;

		oess_2::char_t * m_buff_ptr;

		//! Позиция для записи данных.
		size_t m_write_pos;

		//! Позиция для чтения данных.
		size_t m_read_pos;
};

//! Псевдоним для цепочки блоков.
/*!
	Используется в scattered_block_stream_t.
	\see scattered_block_stream_t::occupy(), scattered_block_stream_t::release().
*/
typedef std::list< raw_block_t * > raw_block_chain_t;

} /* namespace so_5_transport */

#endif
