/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс, для обеспечения mbox-ов мьютексами
	и хранения именованных mbox-ов в словаре.
*/

#if !defined( _SO_5__RT__IMPL__MBOX_CORE_HPP_ )
#define _SO_5__RT__IMPL__MBOX_CORE_HPP_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <ace/Thread_Mutex.h>
#include <ace/RW_Thread_Mutex.h>

#include <so_5/rt/h/atomic_refcounted.hpp>

#include <so_5/util/h/mutex_pool.hpp>

#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/nonempty_name.hpp>

#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class mbox_core_ref_t;

//
// mbox_core_t
//

//! Класс, для обеспечения mbox-ов мьютексами
//! и хранения именованных mbox-ов в словаре.
class mbox_core_t
	:
		private atomic_refcounted_t
{
		friend class mbox_core_ref_t;

		mbox_core_t( const mbox_core_t & );
		void
		operator = ( const mbox_core_t & );

	public:
		explicit mbox_core_t(
			unsigned int mutex_pool_size );
		virtual ~mbox_core_t();

		//! Создать безимянный локальный mbox.
		mbox_ref_t
		create_local_mbox();

		//! Создать именованный локальный mbox.
		mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & nonempty_name );

		//! Создать безимянный локальный mbox,
		//! который использует заданный мутекс.
		mbox_ref_t
		create_local_mbox(
			//! Замок созданный пользователем
			std::unique_ptr< ACE_RW_Thread_Mutex >
				lock_ptr );

		//! Создать именованный локальный mbox,
		//! который использует заданный мутекс.
		mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & nonempty_name,
			//! Замок созданный пользователем
			std::unique_ptr< ACE_RW_Thread_Mutex >
				lock_ptr );

		//! Уничтожить ссылку на именованный mbox.
		/*!
			Если это была последняя ссылка,
			то именованный mbox удаляется.
		*/
		void
		destroy_mbox(
			//! Имя mbox-а.
			const std::string & name );

		//! Взять мутекс в пользование.
		ACE_RW_Thread_Mutex &
		allocate_mutex();

		//! Отказаться от использования мутекса.
		/*!
			Если мутекс \a m не является мутексом из пула,
			то считается, что он создавался отдельно при помощи \c new
			и поэтому будет удален через \c delete.
		*/
		void
		deallocate_mutex( ACE_RW_Thread_Mutex & m );

	private:
		//! Пул мутексов.
		util::mutex_pool_t< ACE_RW_Thread_Mutex > m_mbox_mutex_pool;

		//! Замок для работы со словарем.
		ACE_RW_Thread_Mutex m_dictionary_lock;

		//! Информация о именованном mbox-е.
		struct named_mbox_info_t
		{
			named_mbox_info_t()
				:
					m_external_ref_count( 0 )
			{}

			named_mbox_info_t(
				const mbox_ref_t mbox )
				:
					m_external_ref_count( 1 ),
					m_mbox( mbox )
			{}

			//! Количество ссылок посредством внешних mbox_ref.
			unsigned int m_external_ref_count;
			//! Реальный mbox с помощью которого
			//! идут сооющения.
			mbox_ref_t m_mbox;
		};

		//! Тип для словоря именованных mbox-ов
		typedef std::map< std::string, named_mbox_info_t >
			named_mboxes_dictionary_t;

		//! Словарь именованных mbox-ов.
		named_mboxes_dictionary_t m_named_mboxes_dictionary;
};

//! Класс умной ссылки на mbox_core_t.
class mbox_core_ref_t
{
	public:
		mbox_core_ref_t();

		explicit mbox_core_ref_t(
			mbox_core_t * mbox_core );

		mbox_core_ref_t(
			const mbox_core_ref_t & mbox_core_ref );

		void
		operator = ( const mbox_core_ref_t & mbox_core_ref );

		~mbox_core_ref_t();

		inline const mbox_core_t *
		get() const
		{
			return m_mbox_core_ptr;
		}

		inline mbox_core_t *
		get()
		{
			return m_mbox_core_ptr;
		}

		inline const mbox_core_t *
		operator -> () const
		{
			return m_mbox_core_ptr;
		}

		inline mbox_core_t *
		operator -> ()
		{
			return m_mbox_core_ptr;
		}

		inline mbox_core_t &
		operator * ()
		{
			return *m_mbox_core_ptr;
		}


		inline const mbox_core_t &
		operator * () const
		{
			return *m_mbox_core_ptr;
		}

		inline bool
		operator == ( const mbox_core_ref_t & mbox_core_ref ) const
		{
			return m_mbox_core_ptr ==
				mbox_core_ref.m_mbox_core_ptr;
		}

		inline bool
		operator < ( const mbox_core_ref_t & mbox_core_ref ) const
		{
			return m_mbox_core_ptr <
				mbox_core_ref.m_mbox_core_ptr;
		}

	private:
		//! Увеличить количество ссылок на mbox_core
		//! и в случае необходимости удалить его.
		void
		inc_mbox_core_ref_count();

		//! Уменьшить количество ссылок на mbox_core
		//! и в случае необходимости удалить его.
		void
		dec_mbox_core_ref_count();

		mbox_core_t * m_mbox_core_ptr;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
