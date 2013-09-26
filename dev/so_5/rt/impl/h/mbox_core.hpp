/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of utility class for work with mboxes.
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

#include <so_5/rt/h/mbox.hpp>
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

/*!
 * \brief A utility class for work with mboxes.
 */
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

		//! Create local anonymous mbox.
		mbox_ref_t
		create_local_mbox();

		//! Create local named mbox.
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & nonempty_name );

		//! Create local anonymous mbox with specified mutex.
		mbox_ref_t
		create_local_mbox(
			//! A mutex for mbox.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );

		//! Create local named mbox with specified mutex.
		mbox_ref_t
		create_local_mbox(
			//! Mbox name.
			const nonempty_name_t & nonempty_name,
			//! A mutex for mbox.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );

		//! Remove a reference to named mbox.
		/*!
		 * If it was a last reference to named mbox the mbox destroyed.
		*/
		void
		destroy_mbox(
			//! Mbox name.
			const std::string & name );

		//! Allocate mutex from pool.
		ACE_RW_Thread_Mutex &
		allocate_mutex();

		//! Release mutex.
		/*!
		 * If \a m is a mutex from pool it is returned to pool.
		 * Otherwise it is assumed that \a m is a user supplied mutex and
		 * it is destroyed via delete operator.
		 */
		void
		deallocate_mutex( ACE_RW_Thread_Mutex & m );

	private:
		//! Mutex pool.
		util::mutex_pool_t< ACE_RW_Thread_Mutex > m_mbox_mutex_pool;

		//! Named mbox map's lock.
		ACE_RW_Thread_Mutex m_dictionary_lock;

		//! Named mbox information.
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

			//! Reference count by external mbox_refs.
			unsigned int m_external_ref_count;
			//! Real mbox for that name.
			mbox_ref_t m_mbox;
		};

		//! Typedef for map from mbox name to mbox information.
		typedef std::map< std::string, named_mbox_info_t >
			named_mboxes_dictionary_t;

		//! Named mboxes.
		named_mboxes_dictionary_t m_named_mboxes_dictionary;
};

//! Smart reference to mbox_core_t.
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
		//! Increment reference count to mbox_core.
		void
		inc_mbox_core_ref_count();

		//! Decrement reference count to mbox_core.
		/*!
		 * If reference count become 0 then mbox_core is destroyed.
		 */
		void
		dec_mbox_core_ref_count();

		mbox_core_t * m_mbox_core_ptr;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
