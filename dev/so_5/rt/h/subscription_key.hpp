/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition for subsciption key for mbox.
*/

#if !defined( _SO_5__RT__SUBSCRIPTION_KEY_HPP_ )
#define _SO_5__RT__SUBSCRIPTION_KEY_HPP_

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/type_wrapper.hpp>

namespace so_5
{

namespace rt
{

//! A class for which can be used to indentify message and mbox pair.
struct SO_5_TYPE subscription_key_t
{
	//! Constructor.
	subscription_key_t(
		const type_wrapper_t & type_wrapper,
		const mbox_ref_t & mbox );

	~subscription_key_t();

	//! Operator less than.
	bool
	operator < (
		const subscription_key_t & subscription_key ) const;

	//! Equality operator.
	bool
	operator == (
		const subscription_key_t & subscription_key ) const;

	//! Message type identifier.
	const type_wrapper_t m_type_wrapper;

	//! Reference to mbox.
	const mbox_ref_t m_mbox;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
