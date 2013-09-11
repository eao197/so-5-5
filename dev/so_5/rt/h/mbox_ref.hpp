/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Smart reference for mbox.
*/

#if !defined( _SO_5__RT__MBOX_REF_HPP_ )
#define _SO_5__RT__MBOX_REF_HPP_

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class mbox_t;

//! Smart reference for mbox_t.
class SO_5_TYPE mbox_ref_t
{
	public:
		explicit mbox_ref_t(
			mbox_t * mbox );

		mbox_ref_t();

		mbox_ref_t(
			const mbox_ref_t & mbox_ref );

		mbox_ref_t &
		operator = ( const mbox_ref_t & mbox_ref );

		~mbox_ref_t();

		inline mbox_t *
		get() const
		{
			return m_mbox_ptr;
		}

		inline mbox_t *
		operator -> ()
		{
			return m_mbox_ptr;
		}

		inline const mbox_t *
		operator -> () const
		{
			return m_mbox_ptr;
		}

		inline mbox_t &
		operator * ()
		{
			return * m_mbox_ptr;
		}

		inline const mbox_t &
		operator * () const
		{
			return *m_mbox_ptr;
		}

		void
		release();

		bool
		operator == ( const mbox_ref_t & mbox_ref ) const;

		bool
		operator < ( const mbox_ref_t & mbox_ref ) const;

	private:
		//! Decrement reference count for mbox.
		/*!
		 * Deletes mbox when reference count become 0.
		 */
		void
		dec_mbox_ref_count();

		//! Increment reference count for mbox.
		void
		inc_mbox_ref_count();

		//! Actual mbox.
		mbox_t * m_mbox_ptr;
};

} /* namespace rt */

} /* namespace so_5 */

#endif

