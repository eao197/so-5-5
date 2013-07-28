/*
	SObjectizer 5 Transport.
*/

#include <algorithm>

#include <ace/Synch.h>
#include <ace/OS.h>

#include <so_5_transport/h/scattered_block_stream.hpp>

namespace so_5_transport
{

namespace block_stream_notifiers
{

//
// flush_notifier_t
//

flush_notifier_t::~flush_notifier_t()
{
}

//
// free_blocks_availability_notifier_t
//

free_blocks_availability_notifier_t::~free_blocks_availability_notifier_t()
{
}

//
// stream_operation_notifier_t
//

stream_operation_notifier_t::~stream_operation_notifier_t()
{
}

namespace /* ananymous */
{

//
// noop_flush_notifier_t
//

class noop_flush_notifier_t
	:
		public flush_notifier_t
{
	public:
		virtual void
		notify()
		{}

		virtual void
		reset()
		{}
};

// Нотификатор по умолчанию.
noop_flush_notifier_t noop_flush_notifier;

//
//
//
// noop_free_blocks_availability_notifier_t
//

class noop_free_blocks_availability_notifier_t
	:
		public free_blocks_availability_notifier_t
{
	public:
		virtual void
		notify_lacking_free_blocks()
		{}

		virtual void
		notify_free_blocks(
			size_t default_block_count,
			size_t available_block_count )
		{}
};

// Нотификатор по умолчанию.
noop_free_blocks_availability_notifier_t
	noop_free_blocks_availability_notifier;

class noop_stream_operation_notifier_t
	:
		public stream_operation_notifier_t
{
	public:
		virtual void
		notify_blocks_pushed_back()
		{}

		virtual void
		notify_blocks_pushed_front()
		{}

		virtual void
		notify_blocks_queried()
		{}
};

noop_stream_operation_notifier_t noop_stream_operation_notifier;

} /* ananymous namespace */

} /* namespace block_stream_notifiers */

//
// scattered_block_stream_params_t
//

scattered_block_stream_params_t::scattered_block_stream_params_t()
	:
		m_block_count( 16 ),
		m_max_block_count( 64 ),
		m_single_block_size( 32*1024 )
{
}
scattered_block_stream_params_t::scattered_block_stream_params_t(
	size_t block_count,
	size_t max_block_count,
	size_t single_block_size )
	:
		m_block_count( block_count ),
		m_max_block_count( max_block_count ),
		m_single_block_size( single_block_size )
{
}

//
// scattered_block_stream_t
//

scattered_block_stream_t::scattered_block_stream_t(
	size_t block_count,
	size_t max_block_count,
	size_t single_block_size )
	:
		m_block_count( block_count ),
		m_max_block_count(
			std::max< size_t >(
				max_block_count,
				block_count ) ),
		m_deallocate_block_count_threshold(
			std::min< size_t >(
				m_block_count * 2,
				m_max_block_count ) ),
		m_current_block_count( block_count ),
		m_single_block_size( single_block_size ),
		m_has_free_blocks( m_lock ),
		m_stream_size_in_bytes( 0 ),
		m_flush_notifier( &block_stream_notifiers::noop_flush_notifier ),
		m_free_blocks_availability_notifier(
			&block_stream_notifiers::noop_free_blocks_availability_notifier ),
		m_stream_operation_notifier(
			&block_stream_notifiers::noop_stream_operation_notifier )
{
	for( size_t i = 0; i < m_block_count; ++i )
		m_free_blocks.push_back( new raw_block_t( m_single_block_size ) );
}

scattered_block_stream_t::scattered_block_stream_t(
	const scattered_block_stream_params_t & params )
	:
		m_block_count( params.m_block_count ),
		m_max_block_count(
			std::max< size_t >(
				params.m_max_block_count,
				params.m_block_count ) ),
		m_deallocate_block_count_threshold(
			std::min< size_t >(
				m_block_count * 2,
				m_max_block_count ) ),
		m_current_block_count( params.m_block_count ),
		m_single_block_size( params.m_single_block_size ),
		m_has_free_blocks( m_lock ),
		m_stream_size_in_bytes( 0 ),
		m_flush_notifier( &block_stream_notifiers::noop_flush_notifier ),
		m_free_blocks_availability_notifier(
			&block_stream_notifiers::noop_free_blocks_availability_notifier ),
		m_stream_operation_notifier(
			&block_stream_notifiers::noop_stream_operation_notifier )
{
	for( size_t i = 0; i < m_block_count; ++i )
		m_free_blocks.push_back( new raw_block_t( m_single_block_size ) );
}

scattered_block_stream_t::~scattered_block_stream_t()
{
	std::for_each(
		m_stream_chain.begin(),
		m_stream_chain.end(),
		[]( raw_block_t * block ){
			delete block;
		} );
	std::for_each(
		m_free_blocks.begin(),
		m_free_blocks.end(),
		[]( raw_block_t * block ){
			delete block;
		} );
}

void
scattered_block_stream_t::reset_flush_notifier(
	block_stream_notifiers::flush_notifier_t * flush_notifier )
{
	scoped_lock_t lock( m_lock );
	if( nullptr != flush_notifier )
		m_flush_notifier = flush_notifier;
	else
		m_flush_notifier = &block_stream_notifiers::noop_flush_notifier;
}

void
scattered_block_stream_t::reset_free_blocks_availability_notifier(
	block_stream_notifiers::free_blocks_availability_notifier_t *
		free_blocks_availability_notifier )
{
	scoped_lock_t lock( m_lock );
	if( nullptr != free_blocks_availability_notifier )
		m_free_blocks_availability_notifier = free_blocks_availability_notifier;
	else
		m_free_blocks_availability_notifier =
			&block_stream_notifiers::noop_free_blocks_availability_notifier;
}

void
scattered_block_stream_t::reset_stream_operation_notifier(
	block_stream_notifiers::stream_operation_notifier_t *
		stream_operation_notifier )
{
	scoped_lock_t lock( m_lock );
	if( nullptr != stream_operation_notifier )
		m_stream_operation_notifier = stream_operation_notifier;
	else
		m_stream_operation_notifier =
			&block_stream_notifiers::noop_stream_operation_notifier;
}

raw_block_t *
scattered_block_stream_t::occupy_impl(
	const ACE_Time_Value * abs_value,
	bool & was_waiting_for_free_blocks )
{
	raw_block_t * res = nullptr;

	// Если блоков нет, но их еще можно выделить
	// то выделим их
	if( m_free_blocks.empty() && m_current_block_count < m_max_block_count )
	{
		allocate_blocks();
	}

	// Если блоков все-таки нет и выделить их нельзя,
	// то подождем, если можно.
	if( m_free_blocks.empty() )
	{
		was_waiting_for_free_blocks = true;

		if( abs_value )
			m_has_free_blocks.wait( abs_value );
		else
			m_has_free_blocks.wait();
	}
	else
		was_waiting_for_free_blocks = false;

	// Если блоки есть, то возвращаем первый из них.
	if( !m_free_blocks.empty() )
	{
		res = m_free_blocks.front();
		m_free_blocks.pop_front();

		// приводим блок к виду готовому для записи.
		res->reset_write_pos();
	}
	else
		m_free_blocks_availability_notifier->notify_lacking_free_blocks();

	return res;
}


raw_block_t *
scattered_block_stream_t::occupy(
	bool & was_waiting_for_free_blocks,
	const ACE_Time_Value * timeout )
{
	ACE_Time_Value wait_until = ACE_OS::gettimeofday();

	if( timeout )
		wait_until += *timeout;

	scoped_lock_t lock( m_lock );

	return occupy_impl(
		timeout ? &wait_until : 0,
		was_waiting_for_free_blocks );
}

raw_block_t *
scattered_block_stream_t::occupy(
	const ACE_Time_Value * timeout )
{
	bool stub;
	return occupy( stub, timeout );
}

void
scattered_block_stream_t::occupy(
	size_t n,
	raw_block_chain_t & block_chain,
	bool & was_waiting_for_free_blocks,
	const ACE_Time_Value * timeout )
{
	block_chain.clear();

	// Проверка а возможно ли столько получить?
	if( n > m_max_block_count )
		return;


	ACE_Time_Value wait_until = ACE_OS::gettimeofday();
	if( timeout )
		wait_until += *timeout;
	ACE_Time_Value * abs_wait = timeout? &wait_until : 0;

	was_waiting_for_free_blocks = false;

	scoped_lock_t lock( m_lock );

	while( block_chain.size() < n )
	{
		bool temp_was_waiting_for_free_blocks;
		raw_block_t * block = occupy_impl(
			abs_wait,
			temp_was_waiting_for_free_blocks );

		if( temp_was_waiting_for_free_blocks )
			was_waiting_for_free_blocks = true;

		if( !block )
		{
			release_impl( block_chain );
			block_chain.clear();
			break;
		}
	}
}

void
scattered_block_stream_t::occupy(
	size_t n,
	raw_block_chain_t & block_chain,
	const ACE_Time_Value * timeout )
{
	bool stub;
	occupy( n, block_chain, stub, timeout );
}

void
scattered_block_stream_t::release_impl(
	raw_block_t * block )
{
	if( block )
	{
		m_free_blocks.push_back( block );

		if( m_free_blocks.size() >= m_deallocate_block_count_threshold )
		{
			deallocate_blocks();
		}
	}
}

void
scattered_block_stream_t::release_impl(
	const raw_block_chain_t & chain )
{
	std::for_each(
		chain.begin(),
		chain.end(),
		[ this ]( raw_block_t * block ){
			release_impl( block );
		} );
}

void
scattered_block_stream_t::release( raw_block_t * block )
{
	if( block )
	{
		scoped_lock_t lock( m_lock );

		bool was_empty = m_free_blocks.empty();

		release_impl( block );

		if( was_empty )
			m_has_free_blocks.signal();

		m_free_blocks_availability_notifier
			->notify_free_blocks(
				m_block_count,
				m_free_blocks.size() );
	}
}

void
scattered_block_stream_t::release(
	const raw_block_chain_t & chain )
{
	if( !chain.empty() )
	{
		scoped_lock_t lock( m_lock );
		bool was_empty = m_free_blocks.empty();


		release_impl( chain );

		if( was_empty )
			m_has_free_blocks.signal();

		m_free_blocks_availability_notifier
			->notify_free_blocks(
				m_block_count,
				m_free_blocks.size() );
	}
}

void
scattered_block_stream_t::stream_push_back(
	raw_block_t * block )
{
	if( nullptr != block )
	{
		scoped_lock_t lock( m_lock );

		m_stream_chain.push_back( block );
		m_stream_size_in_bytes += block->read_size_available();

		m_stream_operation_notifier->notify_blocks_pushed_back();
	}
}


void
scattered_block_stream_t::stream_push_back(
	const raw_block_chain_t & chain )
{
	scoped_lock_t lock( m_lock );

	std::for_each(
		chain.begin(),
		chain.end(),
		[ this ]( raw_block_t * block ){
			if( nullptr != block )
			{
				m_stream_chain.push_back( block );
				m_stream_size_in_bytes += block->read_size_available();
			}
		} );

	m_stream_operation_notifier->notify_blocks_pushed_back();
}

void
scattered_block_stream_t::stream_push_front( raw_block_t * block )
{
	if( nullptr != block )
	{
		scoped_lock_t lock( m_lock );

		m_stream_chain.push_front( block );
		m_stream_size_in_bytes += block->read_size_available();

		m_stream_operation_notifier->notify_blocks_pushed_front();
	}
}

void
scattered_block_stream_t::stream_push_front(
	const raw_block_chain_t & chain )
{
	scoped_lock_t lock( m_lock );
	std::for_each(
		chain.rbegin(),
		chain.rend(),
		[ this ]( raw_block_t * block ){
			if( nullptr != block )
			{
				m_stream_chain.push_front( block );
				m_stream_size_in_bytes += block->read_size_available();
			}
		} );

	m_stream_operation_notifier->notify_blocks_pushed_front();
}

void
scattered_block_stream_t::stream_push_front(
	raw_block_chain_t & chain,
	size_t size_in_bytes )
{
	scoped_lock_t lock( m_lock );

	m_stream_chain.splice(
		m_stream_chain.begin(),
		chain );

	m_stream_size_in_bytes += size_in_bytes;

	m_stream_operation_notifier->notify_blocks_pushed_front();
}

size_t
scattered_block_stream_t::stream_query_all_blocks(
	raw_block_chain_t & chain )
{
	chain.clear();

	scoped_lock_t lock( m_lock );

	chain.swap( m_stream_chain );

	const size_t stream_size_in_bytes = m_stream_size_in_bytes;
	m_stream_size_in_bytes = 0;

	m_stream_operation_notifier->notify_blocks_queried();

	return stream_size_in_bytes;
}

size_t
scattered_block_stream_t::stream_query_n_blocks(
	size_t n,
	raw_block_chain_t & chain )
{
	chain.clear();

	size_t obtained_size = 0;

	scoped_lock_t lock( m_lock );

	while( !m_stream_chain.empty() &&
		chain.size() < n )
	{
		raw_block_t * block = m_stream_chain.front();
		m_stream_chain.pop_front();

		obtained_size += block->read_size_available();
		chain.push_back( block );
	}

	m_stream_size_in_bytes -= obtained_size;

	m_stream_operation_notifier->notify_blocks_queried();

	return obtained_size;
}


void
scattered_block_stream_t::flush()
{
	scoped_lock_t lock( m_lock );
	m_flush_notifier->notify();
}

size_t
scattered_block_stream_t::current_block_count() const
{
	scoped_lock_t lock( m_lock );

	return m_current_block_count;
}

bool
scattered_block_stream_t::check_stream_has_blocks()
{
	scoped_lock_t lock( m_lock );

	const bool is_empty = m_stream_chain.empty();
	if( is_empty )
		m_flush_notifier->reset();

	return !is_empty;
}

void
scattered_block_stream_t::touch_stream()
{
	m_stream_operation_notifier->notify_blocks_queried();
}

void
scattered_block_stream_t::allocate_blocks()
{
	const size_t extra_blocks_count = std::min< size_t >(
		m_block_count, m_max_block_count - m_current_block_count );

	for( size_t i = 0; i < extra_blocks_count; ++i )
		m_free_blocks.push_back( new raw_block_t( m_single_block_size ) );

	m_current_block_count += extra_blocks_count;
}

void
scattered_block_stream_t::deallocate_blocks()
{
	if( m_free_blocks.size() > m_block_count )
	{
		size_t release_block_count = m_free_blocks.size() - m_block_count;

		for( size_t i = 0; i < release_block_count; ++i )
		{
			delete m_free_blocks.front();
			m_free_blocks.pop_front();
		}

		m_current_block_count -= release_block_count;
	}
}

//
// scattered_block_stream_ref_t
//

scattered_block_stream_ref_t::scattered_block_stream_ref_t()
	:
		m_scattered_block_stream_ptr( 0 )
{
}

scattered_block_stream_ref_t::scattered_block_stream_ref_t(
	scattered_block_stream_t * scattered_block_stream )
	:
		m_scattered_block_stream_ptr( scattered_block_stream )
{
	inc_scattered_block_stream_ref_count();
}

scattered_block_stream_ref_t::scattered_block_stream_ref_t(
	const scattered_block_stream_ref_t & scattered_block_stream_ref )
	:
		m_scattered_block_stream_ptr( scattered_block_stream_ref.m_scattered_block_stream_ptr )
{
	inc_scattered_block_stream_ref_count();
}

void
scattered_block_stream_ref_t::operator = (
	const scattered_block_stream_ref_t & scattered_block_stream_ref )
{
	if( &scattered_block_stream_ref != this )
	{
		dec_scattered_block_stream_ref_count();

		m_scattered_block_stream_ptr = scattered_block_stream_ref.m_scattered_block_stream_ptr;
		inc_scattered_block_stream_ref_count();
	}

}

scattered_block_stream_ref_t::~scattered_block_stream_ref_t()
{
	dec_scattered_block_stream_ref_count();
}

inline void
scattered_block_stream_ref_t::dec_scattered_block_stream_ref_count()
{
	if( m_scattered_block_stream_ptr &&
		0 == m_scattered_block_stream_ptr->dec_ref_count() )
	{
		delete m_scattered_block_stream_ptr;
		m_scattered_block_stream_ptr = 0;
	}
}

inline void
scattered_block_stream_ref_t::inc_scattered_block_stream_ref_count()
{
	if( m_scattered_block_stream_ptr )
		m_scattered_block_stream_ptr->inc_ref_count();
}

} /* namespace so_5_transport */
