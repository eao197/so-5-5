/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Definition of the template class event_data.
*/

#pragma once

#include <so_5/rt/h/message.hpp>

#include <type_traits>

namespace so_5
{

namespace details
{

/*!
 * \since v.5.5.14
 * \brief A special selector for %mhood_t implementations.
 */
enum class mhood_type_t
{
	//! Message type is a classical message derived from so_5::message_t.
	classical_message,
	//! Message type is a classical signal derived from so_5::signal_t.
	classical_signal,
	//! Message type is not related to so_5::message_t.
	user_type_message
};

/*!
 * \since v.5.5.14
 * \brief A type to be used as a base for %mhood_t implementation.
 */
template< typename M, mhood_type_t T >
class mhood_base_t
{};

/*!
 * \since v.5.5.14
 * \brief A special base for the case when message type is derived
 * from %message_t.
 * 
 * \tparam M type of message.
 */
template< typename M >
class mhood_base_t< M, mhood_type_t::classical_message >
{
public :
	using payload_type = typename message_payload_type< M >::payload_type;
	using envelope_type = typename message_payload_type< M >::envelope_type;

	mhood_base_t( message_ref_t & mf )
		: m_msg{ message_payload_type< M >::extract_payload_ptr( mf ) }
		{}

	//! Access to the message.
	const payload_type *
	get() const { return m_msg; }

	//! Create a smart pointer for the message envelope.
	intrusive_ptr_t< envelope_type >
	make_reference() const { return intrusive_ptr_t< envelope_type >{m_msg}; }

	//! Access to the message.
	const payload_type &
	operator * () const { return *get(); }

	//! Access to the message via pointer.
	const payload_type *
	operator->() const { return get(); }

private :
	M * m_msg;
};

/*!
 * \since v.5.5.14
 * \brief A special base for the case when message type is derived
 * from %message_t.
 * 
 * \tparam M type of signal.
 */
template< typename M >
class mhood_base_t< M, mhood_type_t::classical_signal >
{
public :
	mhood_base_t( message_ref_t & ) {}
};

/*!
 * \since v.5.5.14
 * \brief A special base for the case when message type is not
 * related to %message_t.
 * 
 * \tparam M type of message.
 */
template< typename M >
class mhood_base_t< M, mhood_type_t::user_type_message >
{
public :
	using payload_type = typename message_payload_type< M >::payload_type;
	using envelope_type = typename message_payload_type< M >::envelope_type;

	mhood_base_t( message_ref_t & mf )
		: m_payload{ message_payload_type< M >::extract_payload_ptr( mf ) }
		, m_envelope{ message_payload_type< M >::extract_envelope_ptr( mf ) }
		{}

	//! Access to the message.
	const payload_type *
	get() const { return m_payload; }

	//! Create a smart pointer for the message envelope.
	intrusive_ptr_t< envelope_type >
	make_reference() const { return intrusive_ptr_t< envelope_type >{m_envelope}; }

	//! Access to the message.
	const payload_type &
	operator * () const { return *get(); }

	//! Access to the message via pointer.
	const payload_type *
	operator->() const { return get(); }

private :
	const payload_type * m_payload;
	envelope_type * m_envelope;
};

/*!
 * \since v.5.5.14
 * \brief A special selector of message hood type.
 */
template< typename T >
struct mhood_type_detector
{
	static const mhood_type_t mhood_type =
			is_signal< T >::value ?
					mhood_type_t::classical_signal :
					is_classical_message< T >::value ?
							mhood_type_t::classical_message :
							mhood_type_t::user_type_message;
};

} /* namespace details */

//FIXME: more descriptions and examples needed for mhood_t.
/*!
 * \since v.5.5.14
 * \brief A message wrapped to be used as type of argument for
 * event handlers.
 *
 * \tparam M type of message or signal.
 */
template< typename M >
class mhood_t
	: public details::mhood_base_t< M, details::mhood_type_detector< M >::mhood_type >
{
	using base_type_t = details::mhood_base_t<
			M,
			details::mhood_type_detector< M >::mhood_type >;

public :
	mhood_t( message_ref_t & mf ) : base_type_t{ mf } {}
};

/*!
 * \brief An alias for compatibility with previous versions.
 * \deprecated Will be removed in v.5.6.0.
 */
template< class MSG >
using event_data_t = mhood_t< MSG >;

namespace rt
{

/*!
 * \deprecated Will be removed in v.5.6.0. Use so_5::event_data_t instead.
 */
template< class MSG >
using event_data_t = so_5::event_data_t< MSG >;

} /* namespace rt */

} /* namespace so_5 */

