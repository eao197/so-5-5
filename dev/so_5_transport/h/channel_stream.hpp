/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Класс для оперирования потоком буфферов.
*/
#if !defined( _SO_5_TRANSPORT__CHANNEL_STREAM_HPP_ )
#define _SO_5_TRANSPORT__CHANNEL_STREAM_HPP_

#include <memory>
#include <array>

#include <oess_2/io/h/binstream.hpp>

#include <so_5_transport/h/declspec.hpp>

#include <so_5_transport/h/scattered_block_stream.hpp>

namespace so_5_transport
{

//
// channel_input_stream_t
//

//! Входящий oess_2::io::ibinstream_t поток берущий данные из scattered_block_stream_t.
/*!
	Работа с потоком базируется на понятии транзакции чтения данных.
	С началом транзакции поток извлекает все данные из потока,
	и запоминает сколько байт доступно для чтения.
	После начала транзакции количество еще не прочитанных байт
	доступно через метод bytes_available().

	Если пользователь попытается зачитать больше байт чем
	доступно, то будет пораждено исключение, а транзакция
	чтения будет отменена.

	После того как из потока зачитаны все данные, которые
	сочтено возможным зачитать, надо подтвердить транзакцию
	через метод commit_transaction().

	После подтверждения или отмены транзакции поток не содержит
	никаких данных, а попытка чтения приводит к исключению.
*/
class SO_5_TRANSPORT_TYPE channel_input_stream_t
	:
		public oess_2::io::ibinstream_t
{
		channel_input_stream_t(
			const channel_input_stream_t & );
		void
		operator = (
			const channel_input_stream_t & );

	public:
		channel_input_stream_t(
			const scattered_block_stream_ref_t & scattered_block_stream );
		virtual ~channel_input_stream_t();

		//! Забирает из потока блоков
		//! все блоки и организует поток чтения над ними.
		/*!
			При начале транзакция фиксируетя начальная контрольная точка.
		*/
		void
		begin_transaction();

		//! Установить контрольную точку (промежуточное подтверждение транзакции).
		/*!
			Подтверждает зачитанные результаты, но
			позволяет и дальше работать с потоком.
			Если после этого будет иницирована отмена транзакции,
			то она откатиться к состоянию зафиксированому
			последней контрольной точкой.
		*/
		void
		checkpoint();

		//! Подтвердить транзакцию чтения данных.
		void
		commit_transaction();

		//! Отменить транзакцию чтения данных.
		/*!
			Отменяет все операции чтения в рамках транзакции
			и кладет их обратно в поток.
		*/
		void
		rollback_transaction();

		//! Количество доступных в потоке байт.
		size_t
		bytes_available() const;

		//! Достигнут ли конец потока.
		virtual bool
		eof() const;

	private:
		//! \name Перегрузка методов базового класса.
		//! \{

		//! Метод для определения указателя, из которого
		//! можно считывать данные.
		virtual const oess_2::char_t *
		in_reserve(
			//! Количество элементов, которые нужно
			//! прочитать.
			size_t item_count,
			//! Размер одного элемента в байтах.
			size_t item_size,
			//! Количество элементов, доступных по
			//! возвращенному указателю.
			size_t & item_available );

		//! Вспомогательный метод для копирования
		//! во внутренний буффер элемента, который
		//! находится в 2-ух блоках.
		void
		copy_devided_item_to_internal_buffer(
			size_t item_size );

		//! Сдвинуть указатель.
		virtual void
		in_shift(
			//! Количество успешно прочитанных элементов.
			size_t item_count,
			//! Размер одного элемента в байтах.
			size_t item_size );
		//! \}

		//! Источник блоков.
		scattered_block_stream_ref_t m_block_source;

		//! \name Сведения о транзакции.
		//! \{

		// Отчистить данные о транзакции.
		void
		reset_transaction_data();

		//! Количество байт которое изначально было доступно в потоке.
		size_t m_bytes_initially_available;

		//! Количество доступных для чтения в потоке.
		size_t m_bytes_available;

		//! Начальная позиция первого блока.
		/*!
			Если транзакцию надо будет отменить,
			то первый блок должен иметь ту же позицию в буфере.
		*/
		size_t m_first_block_initial_pos;

		//! Текущий блок.
		raw_block_t * m_current_block;

		//! Блоки которые уже вычитаны,
		//! но транзакция еще не подтверждена.
		raw_block_chain_t m_touched_list;

		//! Блоки которые еще не вычитаны в рамках текущей транзакции.
		raw_block_chain_t m_untouched_list;
		//! \}

		/*!
			Внутренний буфер для данных, которые располагаются на
			границе 2-ух блоков.

			\note В данный момент oess_2::io::ibinstream_t
			зачитывает данные таких типов, что размер одного элемента
			этих типов не превышает 8 байт (Например double или long int),
			поэтому буфера в 16 байт достаточно для хранения элемента
			любого типа, для которого определено чтение через oess_2::io::ibinstream_t.
			Если со временем в oess_2::io::ibinstream_t добавится чтение типа
			который не вмещается в 16 байт то размер буфера надо будет менять.
		*/
		std::array< oess_2::char_t, 16 > m_internal_buf;
};

//! Псевдоним unique_ptr для channel_input_stream_t.
typedef std::unique_ptr< channel_input_stream_t >
	channel_input_stream_unique_ptr_t;

//
// channel_output_stream_t
//

//! Исходящий oess_2::io поток вставляющий данные в scattered_block_stream_t.
/*!

*/
class SO_5_TRANSPORT_TYPE channel_output_stream_t
	:
		public oess_2::io::obinstream_t
{
	public:

		channel_output_stream_t(
			const scattered_block_stream_ref_t & scattered_block_stream,
			//! Сколько времени можно потратить на ожидание
			//! свободных блоков.
			const ACE_Time_Value & free_blocks_awaiting_timeout );

		virtual ~channel_output_stream_t();

		//! Начать транзакцию записи данных.
		/*!
			Приводит поток в состояние готовности к записи данных.
			\note Если в момент начала на потоке происходила транзакция,
			то она отменяется и начаинается новая.
		*/
		void
		begin_transaction();

		//! Установить контрольную точку (промежуточное подтверждение транзакции).
		/*!
			Вставляет накопленные блоки с записанными данными в поток,
			при этом позволяет продолжать транзакцию и записывать
			новые порции данных.
			Если после этого будет иницирована отмена транзакции,
			то она откатиться к состоянию зафиксированому
			последней контрольной точкой.
		*/
		void
		checkpoint();

		//! Подтвердить транзакцию записи данных.
		/*!
			Вставляет накопленные блоки с записанными данными в поток.
		*/
		void
		commit_transaction();

		//! Отменить транзакцию записи данных.
		/*!
			Отменяет все операции записи в рамках транзакции
			и освобождает блоки.
		*/
		void
		rollback_transaction();

		//! Сколько байт записано в поток.
		/*!
			\note Содержит актуальное значение только
			если транзакция активна.
		*/
		size_t
		bytes_written() const;

	private:
		//! Сбросить данные готовые к отправке.
		void
		flush();

		//! Получить очередной блок для записи в него данных.
		void
		occupy_next_block();

		//! Освободить имеющиеся блоки.
		void
		release_blocks();

		//! \name Перегрузка методов базового класса.
		//! \{

		//! Метод для определения указателя, в который
		//! можно записывать данные.
		/*!
			Если нет достаточного объема данных для чтения
			хотя бы одного элемента, то должно порождаться
			исключение.
		*/
		virtual oess_2::char_t *
		out_reserve(
			//! Количество элементов, которые нужно
			//! записать.
			size_t item_count,
			//! Размер одного элемента в байтах.
			size_t item_size,
			//! Количество элементов, доступных для записи по
			//! возвращенному указателю.
			size_t & item_available );

		//! Сдвинуть указатель.
		/*!
			Вызывается после успешной
			записи в буфер, указатель на который был
			возвращен методом out_reserve.
		*/
		virtual void
		out_shift(
			//! Количество успешно записанных элементов.
			size_t item_count,
			//! Размер одного элемента в байтах.
			size_t item_size );
		//! \}

		//! Флаг-состояние: находтся ли поток в процессе выполнения транзакции.
		bool m_trx_running;

		//! Количество байт записанных в поток.
		size_t m_bytes_written;

		//! откуда брать свободные блоки и поток куда вставлять данные.
		scattered_block_stream_ref_t m_block_destination;

		//! Сколько времени можно ожидать прихода новых блоков.
		ACE_Time_Value m_free_blocks_awaiting_timeout;

		//! Текущий блок.
		raw_block_t * m_current_block;

		//! Блоки которые записаны, но не отправлены.
		raw_block_chain_t m_ready_blocks;

		/*!
			Внутренний буфер для записи данных, которые
			не могут быть записаны в один блок.

			\note В данный момент oess_2::io::obinstream_t
			записывает данные таких типов, что размер одного элемента
			этих типов не превышает 8 байт (Например double или long int),
			поэтому буфера в 16 байт достаточно для записи элемента
			любого типа, для которого определена запись через
			oess_2::io::obinstream_t. Если со временем в
			oess_2::io::obinstream_t добавится запись типа
			который не вмещается в 16 байт то размер буфера
			надо будет увеличить.
		*/
		std::array< oess_2::char_t, 16 > m_internal_buf;
};

//! Псевдоним unique_ptr для channel_output_stream_t.
typedef std::unique_ptr< channel_output_stream_t >
	channel_output_stream_unique_ptr_t;

} /* namespace so_5_transport */

#endif
