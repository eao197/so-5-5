/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс, для обеспечения пула мьютексов.
*/

#if !defined( _SO_5__UTIL__MUTEX_POOL_HPP_ )
#define _SO_5__UTIL__MUTEX_POOL_HPP_

#include <vector>

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>

namespace so_5
{

namespace util
{

//
// mutex_pool_t
//

//! Класс для реализации пула мутексов заданного типа.
template < class MUTEX >
class mutex_pool_t
{
	public:
		explicit mutex_pool_t(
			unsigned int pool_size )
			:
				m_mutex_pool_size( pool_size ),
				m_mutex_pool( new MUTEX[ m_mutex_pool_size ] ),
				m_mutex_pool_charge( m_mutex_pool_size, 0 )
		{}
		~mutex_pool_t()
		{
			delete [] m_mutex_pool;
		}

		//! Взять мутекс в пользование.
		MUTEX &
		allocate_mutex()
		{
			ACE_Guard< ACE_Thread_Mutex > lock( m_mutex_lock );

			size_t min_index = std::min_element(
				m_mutex_pool_charge.begin(),
				m_mutex_pool_charge.end() ) - m_mutex_pool_charge.begin();

			++m_mutex_pool_charge[ min_index ];
			return m_mutex_pool[ min_index ];
		}

		//! Отказаться от использования мутекса.
		/*!
			\return Если мутекс принадлежит пулу,
			то уменьшает количество ссылок на него и возвращает
			true, в противном случае вернет false и ничего
			не сделает.
		*/
		bool
		deallocate_mutex( MUTEX & m )
		{
			// Если метекс из пула, то будем уменьшать
			// количество ссылок на него.
			if( &m >= m_mutex_pool &&
				&m < m_mutex_pool + m_mutex_pool_size )
			{
				ACE_Guard< ACE_Thread_Mutex > lock( m_mutex_lock );
				const size_t mutex_index =  &m - m_mutex_pool;

				--m_mutex_pool_charge[ mutex_index ];
				return true;
			}

			// Если мутекс не наш, то:
			return false;
		}

	private:
		//! Замок для выдачи мутексов.
		ACE_Thread_Mutex m_mutex_lock;

		//! Размер пула мутексов.
		const size_t m_mutex_pool_size;

		MUTEX * const m_mutex_pool;

		//! Массив для хранения количества ссылок на мутексы.
		std::vector< unsigned int > m_mutex_pool_charge;
};

} /* namespace util */

} /* namespace so_5 */

#endif
