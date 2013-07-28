/*
	SObjectizer 5 Transport.
*/

#include <cstring>
#include <algorithm>

#include <ace/OS.h>

#include <so_5/h/exception.hpp>

#include <so_5_transport/h/ret_code.hpp>
#include <so_5_transport/h/channel_stream.hpp>

namespace so_5_transport
{

channel_input_stream_t::channel_input_stream_t(
	const scattered_block_stream_ref_t & scattered_block_stream )
	:
		m_block_source( scattered_block_stream )
{
	reset_transaction_data();
}

channel_input_stream_t::~channel_input_stream_t()
{
	// Если что-то осталось не закомиченным,
	// то оно вернется в поток.
	rollback_transaction();
}

void
channel_input_stream_t::begin_transaction()
{
	// Если какая-то транзакция и имела место,
	// она отменяется.
	rollback_transaction();

	// Получаем все блоки имеющиеся в потоке.
	m_bytes_initially_available = m_bytes_available =
		m_block_source->stream_query_all_blocks( m_untouched_list );

	if( !m_untouched_list.empty() )
	{
		m_current_block = m_untouched_list.front();
		m_untouched_list.pop_front();

		// При отмене транзакции позицию в первом блоке необходимо
		// востанавливать, поютому запоминаем исходную позицию первого блока.
		m_first_block_initial_pos = m_current_block->read_pos();
	}
	else
	{
		// Читать оказалось нечего, текущий блок отсутствует.
		m_current_block = 0;
	}
}

void
channel_input_stream_t::checkpoint()
{
	// Если текущий блок есть.
	if( m_current_block )
	{
		// Если из текущего блока не все считали.
		if( m_current_block->read_size_available() > 0 )
		{
			// И он не до конца обработан,
			// то добавляем его в начало списка нетронутых.
			m_untouched_list.push_front(
				m_current_block );
		}
		else
		{
			// Иначе добавляем его к тронутым.
			m_touched_list.push_back(
				m_current_block );
		}
		m_current_block = 0;
	}

	// Освобождаем тронутые блоки.
	// А нетронутые остаются для продолжения транзакции.
	m_block_source->release( m_touched_list );
	m_touched_list.clear();

	// Изначально доступное количество байт
	// это ровно то, что на данный момент осталось.
	m_bytes_initially_available = m_bytes_available;

	if( !m_untouched_list.empty() )
	{
		m_current_block = m_untouched_list.front();
		m_untouched_list.pop_front();

		// При отмене транзакции позицию в первом блоке необходимо
		// востанавливать, поютому запоминаем исходную позицию первого блока.
		m_first_block_initial_pos = m_current_block->read_pos();
	}
	else
	{
		// Читать оказалось нечего, текущий блок отсутствует.
		m_current_block = 0;
	}
}

void
channel_input_stream_t::commit_transaction()
{
	// Если текущий блок есть.
	if( m_current_block )
	{
		// Если из текущего блока не все считали.
		if( m_current_block->read_size_available() > 0 )
		{
			// И он не до конца обработан,
			// то добавляем его в начало списка нетронутых.
			m_untouched_list.push_front(
				m_current_block );
		}
		else
		{
			// Иначе добавляем его к тронутым.
			m_touched_list.push_back(
				m_current_block );
		}
	}

	// Освобождаем тронутые блоки.
	m_block_source->release( m_touched_list );

	// Нетронутые блоки добавляем обратно в начало потока.
	// Причем первый блок может иметь позцию чтения не с начала блока.
	m_block_source->stream_push_front( m_untouched_list, m_bytes_available );

	reset_transaction_data();
}

void
channel_input_stream_t::rollback_transaction()
{
	// Добавляем текущий к нетронутым, если он есть.
	if( m_current_block )
	{
		m_current_block->reset_read_pos();
		m_untouched_list.push_front(
			m_current_block );
	}

	// Сцепляем списки: тронутые ++ нетронутые.
	m_untouched_list.splice(
		m_untouched_list.begin(),
		m_touched_list );

	if( !m_untouched_list.empty() )
	{
		// Первому блоку надо выставить указатель чтения
		// таким как мы его получили.
		m_untouched_list.front()->shift_read_pos(
			m_first_block_initial_pos );
	}

	m_block_source->stream_push_front(
		m_untouched_list,
		m_bytes_initially_available );

	reset_transaction_data();
}

size_t
channel_input_stream_t::bytes_available() const
{
	return m_bytes_available;
}

bool
channel_input_stream_t::eof() const
{
	return m_bytes_available == 0;
}

const oess_2::char_t *
channel_input_stream_t::in_reserve(
	size_t item_count,
	size_t item_size,
	size_t & item_available )
{
	if( m_bytes_available < item_size )
	{
		// Отменяем транзакцию.
		rollback_transaction();

		throw so_5::exception_t(
			"not enough data in buffer to read",
			rc_not_enough_data_to_read_from_buffer );
	}

	// Если нет места для прочтения хотябы одного элемента.
	// тогда положим один элемент во внутренний буфер,
	// и отдадим его.
	if( m_current_block->read_size_available() < item_size )
	{
		copy_devided_item_to_internal_buffer( item_size );
		item_available = 1;
		return m_internal_buf.data();
	}

	item_available = std::min< size_t >(
		m_current_block->read_size_available() / item_size,
		item_count );

	return m_current_block->current_read_ptr();
}

void
channel_input_stream_t::copy_devided_item_to_internal_buffer(
	size_t item_size )
{
	// Сколько байт осталось скопировать.
	size_t bytes_to_copy = item_size;

	// Текущий блок откуда копировать данные.
	raw_block_t * block = m_current_block;

	// Итератор на следующий блок.
	raw_block_chain_t::iterator it_next = m_untouched_list.begin();

	// Куда писать данные.
	oess_1::char_t * write_ptr = m_internal_buf.data();

	while( bytes_to_copy > 0 )
	{
		const size_t bytes_to_take_out_of_current_block =
			std::min< size_t >(
				block->read_size_available(),
				bytes_to_copy );

		std::memcpy(
			write_ptr,
			block->current_read_ptr(),
			bytes_to_take_out_of_current_block );

		bytes_to_copy -= bytes_to_take_out_of_current_block;
		write_ptr += bytes_to_take_out_of_current_block;

		// Если еще что-то осталось вычитать.
		// Берем следующий блок.
		if( bytes_to_copy > 0 )
			block = *(it_next++);
	}
}

void
channel_input_stream_t::in_shift(
	size_t item_count,
	size_t item_size )
{
	size_t total_size = item_count * item_size;
	m_bytes_available -= total_size;


	while( total_size &&
		total_size >= m_current_block->read_size_available() )
	{
		// Отмечаем сколько было зачитано
		// из текущего блока.
		total_size -= m_current_block->read_size_available();

		// Сбрасываем результаты чтения.
		// Это дает право при отмене транзакции
		// вернуть в поток блоки в непрочитанном состоянии.
		// Позиция в первом блоке выставится при отмене транзакции.
		m_current_block->reset_read_pos();

		m_touched_list.push_back( m_current_block );

		// Если есть еще блоки, то берем следующий.
		if( !m_untouched_list.empty() )
		{
			m_current_block = m_untouched_list.front();
			m_untouched_list.pop_front();
		}
		else
			m_current_block = 0;
	}

	// Если это еще не конец, то сдвигаем указатель чтения данного
	// блока.
	if( m_current_block )
		m_current_block->shift_read_pos( total_size );

}

void
channel_input_stream_t::reset_transaction_data()
{
	m_bytes_initially_available = 0;
	m_bytes_available = 0;
	m_first_block_initial_pos = 0;
	m_current_block = 0;
	m_touched_list.clear();
	m_untouched_list.clear();
}


//
// channel_output_stream_t
//

channel_output_stream_t::channel_output_stream_t(
	const scattered_block_stream_ref_t & scattered_block_stream,
	const ACE_Time_Value & free_blocks_awaiting_timeout )
	:
		m_trx_running( false ),
		m_block_destination( scattered_block_stream ),
		m_free_blocks_awaiting_timeout( free_blocks_awaiting_timeout ),
		m_current_block( 0 )
{
}

channel_output_stream_t::~channel_output_stream_t()
{
	rollback_transaction();
}

void
channel_output_stream_t::begin_transaction()
{
	// Отменяем текущую транзакцию, если она имеет место.
	if( m_trx_running )
		rollback_transaction();

	m_trx_running = true;
	m_bytes_written = 0;
}

void
channel_output_stream_t::checkpoint()
{
	if( m_trx_running )
	{
		flush();
		m_bytes_written = 0;
	}
}

void
channel_output_stream_t::commit_transaction()
{
	if( m_trx_running )
	{
		flush();
		m_trx_running = false;
	}
}

void
channel_output_stream_t::rollback_transaction()
{
	if( m_trx_running )
	{
		release_blocks();
		m_trx_running = false;
	}
}

size_t
channel_output_stream_t::bytes_written() const
{
	return m_bytes_written;
}

void
channel_output_stream_t::flush()
{
	if( m_current_block != 0 )
	{
		// К готовым блокам добавляем
		// текущий.

		m_ready_blocks.push_back( m_current_block );
		m_current_block = 0;
	}

	if( !m_ready_blocks.empty() )
	{
		m_block_destination->stream_push_back( m_ready_blocks );
		m_block_destination->flush();

		m_ready_blocks.clear();
	}
}

void
handle_trx_is_not_running()
{
	throw so_5::exception_t(
		"transaction isn't started",
		rc_transactiorn_not_started );
}

void
handle_no_free_blocks_available()
{
	throw so_5::exception_t(
		"no free blocks available",
		rc_no_free_blocks_available_in_buffer );
}


void
channel_output_stream_t::occupy_next_block()
{
	if( !m_trx_running )
		handle_trx_is_not_running();

	if( m_current_block )
	{
		m_ready_blocks.push_back( m_current_block );
		m_current_block = 0;
	}

	const ACE_Time_Value before = ACE_OS::gettimeofday();

	bool was_waiting_for_free_blocks;
	m_current_block = m_block_destination->occupy(
		was_waiting_for_free_blocks,
		&m_free_blocks_awaiting_timeout );

	const ACE_Time_Value after = ACE_OS::gettimeofday();

	// Если имело место ожидание свободных блоков,
	// то исключаем время потраченное на ожидание.
	if( was_waiting_for_free_blocks )
	{
		const ACE_Time_Value duration = after - before;

		if( m_free_blocks_awaiting_timeout > duration )
			m_free_blocks_awaiting_timeout -= duration;
		else
			m_free_blocks_awaiting_timeout = ACE_Time_Value::zero;
	}

	if( !m_current_block )
		handle_no_free_blocks_available();
}

void
channel_output_stream_t::release_blocks()
{
	if( m_current_block )
	{
		m_ready_blocks.push_back( m_current_block );
		m_current_block = 0;
	}

	m_block_destination->release( m_ready_blocks );
	m_ready_blocks.clear();
}

oess_1::char_t *
channel_output_stream_t::out_reserve(
	size_t item_count,
	size_t item_size,
	size_t & item_available )
{
	// Если некуда писать, то надо выделить блок.
	if( !m_current_block ||
		0 == m_current_block->write_size_available() )
	{
		occupy_next_block();
	}

	if( m_current_block->write_size_available() < item_size )
	{
		// Если в текущем блоке недостаточно места для записи
		// одного элемента, то запишем его в буфер.
		// откуда часть элемента допишется в текущий блок,
		// а другая часть запишется в следующий блок
		item_available = 1;
		return m_internal_buf.data();
	}

	item_available = std::min< size_t >(
		m_current_block->write_size_available() / item_size,
		item_count );

	return m_current_block->current_write_ptr();
}

void
channel_output_stream_t::out_shift(
	size_t item_count,
	size_t item_size )
{
	m_bytes_written += item_count * item_size;

	if( m_current_block->write_size_available() < item_size )
	{
		// Значит только что запись пошла в буфер
		// и был это только 1 элемент.
		size_t bytes_to_write = item_size;
		oess_2::char_t * src = m_internal_buf.data();

		while( bytes_to_write > 0 )
		{
			size_t how_many_can_i_write =
				std::min< size_t >(
					m_current_block->write_size_available(),
					bytes_to_write );

			std::memcpy(
				m_current_block->current_write_ptr(),
				src,
				how_many_can_i_write );

			m_current_block->shift_write_pos( how_many_can_i_write );

			// Отнимаем сколько мы зачитали.
			bytes_to_write -= how_many_can_i_write;
			// сдвигаем указатель на величину которую считали.
			src += how_many_can_i_write;

			// Этот блок отработан, если мы еще не все записали,
			// то получим следующий блок.
			if( bytes_to_write > 0 )
				occupy_next_block();
		}
	}
	else
	{
		// Запись шла в текущий блок.
		m_current_block->shift_write_pos( item_count*item_size );
	}
}


} /* namespace so_5_transport */
