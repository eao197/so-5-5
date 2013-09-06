/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Base class for object with reference counting definition.
*/

#if !defined( _SO_5__RT__ATOMIC_REFCOUNTED_HPP_ )
#define _SO_5__RT__ATOMIC_REFCOUNTED_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/types.hpp>

namespace so_5
{

namespace rt
{

//! Base class for object with reference counting.
/*!
 * Should be used as base class. Smart reference for such objects
 * should be defined for derived classes (for example so_5::rt::agent_ref_t).
*/
class SO_5_TYPE atomic_refcounted_t
{
		/*! Disabled. */
		atomic_refcounted_t(
			const atomic_refcounted_t & );

		/*! Disabled. */
		atomic_refcounted_t &
		operator = (
			const atomic_refcounted_t & );

	public:
		//! Default constructor
		/*!
		 * Sets reference counter to 0.
		 */
		atomic_refcounted_t();

		//! Destructor.
		/*!
		 * Do nothing.
		 */
		~atomic_refcounted_t();

		//! Increment reference count.
		inline void
		inc_ref_count()
		{
			++m_ref_counter;
		}

		//! Decrement reference count.
		/*!
		 * \return Value of reference counter *after* decrement.
		*/
		inline unsigned long
		dec_ref_count()
		{
			return --m_ref_counter;
		}

	private:
		//! Object reference count.
		atomic_counter_t m_ref_counter;
};

} /* namespace rt */

} /* namespace so_5 */

#endif

