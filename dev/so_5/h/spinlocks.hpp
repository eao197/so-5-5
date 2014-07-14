/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \file
 * \brief Definition of various types of spinlocks.
 */

#if !defined( SO_5__SPINLOCKS_HPP )
#define SO_5__SPINLOCKS_HPP

#include <atomic>
#include <thread>
#include <cstdint>

namespace so_5
{

//
// yield_backoff_t
//
/*!
 * \since v.5.4.0
 * \brief A implementation of backoff object with usage of std::yield.
 */
class yield_backoff_t
	{
	public :
		inline void
		operator()()
			{
				std::this_thread::yield();
			}
	};

//
// empty_backoff_t
//
/*!
 * \since v.5.4.0
 * \brief A implementation of backoff object with no operations inside.
 */
class empty_backoff_t
	{
	public :
		inline void
		operator()()
			{
			}
	};

//
// spinlock_t
//
/*!
 * \since v.5.4.0
 * \brief A simple spinlock (analog of std::mutex).
 *
 * This implemetation in based on description of std::atomic_flag:
 * \see http://en.cppreference.com/w/cpp/atomic/atomic_flag
 */
template< class BACKOFF >
class spinlock_t
	{
	public :
		spinlock_t()
			{
				m_flag.clear( std::memory_order_release );
			}
		spinlock_t( const spinlock_t & ) = delete;
		spinlock_t( spinlock_t && ) = delete;

		spinlock_t & operator=( const spinlock_t & ) = delete;
		spinlock_t & operator=( spinlock_t && ) = delete;

		//! Lock object.
		void
		lock()
			{
				BACKOFF backoff;
				while( m_flag.test_and_set( std::memory_order_acquire ) )
					backoff();
			}

		//! Unlock object.
		void
		unlock()
			{
				m_flag.clear( std::memory_order_release );
			}

	private :
		//! Atomic flag which is used as actual lock.
		std::atomic_flag m_flag;
	};

//
// default_spinlock_t
//
typedef spinlock_t< empty_backoff_t > default_spinlock_t;

//
// rw_spinlock_t
//
/*!
 * \since v.5.4.0
 * \brief A simple multi-readers/single-writer spinlock
 * (analog of std::shared_mutex).
 *
 * This implemetation in based on cds::RWSpinRPrefT class from
 * libcds library (http://libcds.sourceforge.net/).
 *
 * The original code of cds::RWSpinRPrefT can be found in
 * Svn repository:
 * https://sourceforge.net/p/libcds/code/986/tree/tags/Release_1.5.0/cds/lock/rwlock.h
 */
template< class BACKOFF >
class rw_spinlock_t
	{
	private :
		std::atomic_uint_fast32_t m_counters;

		static const std::uint_fast32_t writter_bit = 0x80000000u;

		inline std::uint_fast32_t
		inc_only_readers( std::uint_fast32_t v )
			{
				return (v & ~writter_bit) + 1;
			}

		inline std::uint_fast32_t
		dec_readers( std::uint_fast32_t v )
			{
				auto w = v & writter_bit;
				return ((v & ~writter_bit) - 1) | w;
			}

	public :
		rw_spinlock_t()
			{
				m_counters.store( 0, std::memory_order_release );
			}
		rw_spinlock_t( const rw_spinlock_t & ) = delete;
		rw_spinlock_t( rw_spinlock_t && ) = delete;

		rw_spinlock_t & operator=( const rw_spinlock_t & ) = delete;
		rw_spinlock_t & operator=( rw_spinlock_t && ) = delete;

		//! Lock object in shared mode.
		inline void
		lock_shared()
			{
				BACKOFF backoff;

				std::uint_fast32_t expected =
					m_counters.load( std::memory_order_relaxed );
				std::uint_fast32_t desired;

				while( true )
					{
						desired = inc_only_readers( expected );
						if( m_counters.compare_exchange_weak(
								expected, desired,
								std::memory_order_release,
								std::memory_order_relaxed ) )
							break;

						backoff();
					}
			}

		//! Unlock object locked in shared mode.
		inline void
		unlock_shared()
			{
				BACKOFF backoff;

				std::uint_fast32_t expected =
					m_counters.load( std::memory_order_relaxed );
				std::uint_fast32_t desired;

				while( true )
					{
						desired = dec_readers( expected );
						if( m_counters.compare_exchange_weak(
								expected, desired,
								std::memory_order_release,
								std::memory_order_relaxed ) )
							break;

						backoff();
					}
			}

		//! Lock object in exclusive mode.
		inline void
		lock()
			{
				BACKOFF backoff;

				std::uint_fast32_t expected;
				std::uint_fast32_t desired;

				while( true )
					{
						expected = 0;
						desired = writter_bit;
						if( m_counters.compare_exchange_weak(
								expected, desired,
								std::memory_order_release,
								std::memory_order_relaxed ) )
							break;

						backoff();
					}
			}

		//! Unlock object locked in exclusive mode.
		inline void
		unlock()
			{
				m_counters.store( 0, std::memory_order_release );
			}
	};

typedef rw_spinlock_t< empty_backoff_t > default_rw_spinlock_t;

//
// read_lock_guard_t
//
/*!
 * \since v.5.4.0
 * \brief Scoped guard for shared locks.
 */
template< class LOCK >
class read_lock_guard_t
	{
	private :
		LOCK & m_lock;

	public :
		read_lock_guard_t( LOCK & l ) : m_lock( l )
			{
				m_lock.lock();
			}
		~read_lock_guard_t()
			{
				m_lock.unlock();
			}

		read_lock_guard_t( const read_lock_guard_t & ) = delete;
		read_lock_guard_t( read_lock_guard_t && ) = delete;
	};

} /* namespace so_5 */

#endif

