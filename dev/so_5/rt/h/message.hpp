/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A base class for agent messages definition.
*/

#if !defined( _SO_5__RT__MESSAGE_HPP_ )
#define _SO_5__RT__MESSAGE_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/exception.hpp>
#include <so_5/rt/h/atomic_refcounted.hpp>

#include <type_traits>

namespace so_5
{

namespace rt
{

//
// message_t
//

//! A base class for agent messages.
/*!
 * All messages for agents should be derived from that class.
 *
 * \attention This class should be used for all messages those
 * have actual message data. For signals (messages without any data)
 * a signal_t class should be used as base class.
 */
class SO_5_TYPE message_t
	:
		private atomic_refcounted_t
{
		friend class message_ref_t;

	public:
		message_t();
		message_t( const message_t & );
		void
		operator = ( const message_t & );

		virtual ~message_t();
};

//
// signal_t
//
//! A base clas for agent signals.
/*!
 * \since v.5.2.0
 *
 * All signals (messages without any data) for agents should be
 * derived from that class.
 */
class SO_5_TYPE signal_t
	:	public message_t
{
	private :
		/*!
		 * Private constructor to disable creation of instances of
		 * derived classes.
		 */
		signal_t();

	public :
		virtual ~signal_t();
};

//
// ensure_not_signal
//
/*!
 * \since v.5.2.0
 * \brief A special compile-time checker to guarantee that message
 * class is not a signal class.
 */
template< class MSG >
void
ensure_not_signal()
{
	static_assert( !std::is_base_of< signal_t, MSG >::value,
			"instance of signal_t cannot be used in place of instance of "
			"message_t" );
	static_assert( std::is_base_of< message_t, MSG >::value,
			"message class should be derived from message_t" );
}

//
// ensure_message_with_actual_data
//
/*!
 * \since v.5.2.0
 * \brief A special checker to guarantee that message is an instance
 * of message_t (not signal_t) and has not-null pointer to message data.
 *
 * \note A check for inheritance from message_t is done at compile-time.
 *
 * \tparam MSG message type to be checked.
 */
template< class MSG >
void
ensure_message_with_actual_data( const MSG * m )
{
	ensure_not_signal< MSG >();

	if( !m )
		throw so_5::exception_t(
				"an attempt to send message via nullptr",
				so_5::rc_null_message_data );
}

//
// ensure_signal
//
/*!
 * \since v.5.2.0
 * \brief A special compile-time checker to guarantee that MSG is derived
 * from signal_t.
 *
 * \tparam MSG signal type to be checked.
 */
template< class MSG >
void
ensure_signal()
{
	static_assert( std::is_base_of< signal_t, MSG >::value,
			"expected a type derived from signal_t" );
}

} /* namespace rt */

} /* namespace so_5 */

#endif

