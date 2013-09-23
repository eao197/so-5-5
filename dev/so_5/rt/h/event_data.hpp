/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Definition of template class event_data.
*/


#if !defined( _SO_5__RT__EVENT_DATA_HPP_ )
#define _SO_5__RT__EVENT_DATA_HPP_

#include <so_5/rt/h/message.hpp>

#include <so_5/rt/h/mbox_ref.hpp>

#include <type_traits>

namespace so_5
{

namespace rt
{

class mbox_t;

/*!
 * \since v.5.2.0
 * \brief A special base for %event_data_t template.
 *
 * This base has two specialization: for signals (without any facilities
 * to access message data) and for messages (with facilities to
 * access actual message data).
 *
 * \tparam MSG type of message or signal.
 * \tparam IS_SIGNAL a flag which should be \a true for signals and
 * \a false for messages.
 */
template< class MSG, bool IS_SIGNAL >
class event_data_base_t {};

/*!
 * \since v.5.2.0
 *
 * A specialization for signal_t has no methods to access message data.
 */
template< class MSG >
class event_data_base_t< MSG, true >
{
	public :
		event_data_base_t( const MSG * ) {}
};

/*!
 * \since v.5.2.0
 *
 * A specialization for message_t has all methods to access message data.
 */
template< class MSG >
class event_data_base_t< MSG, false >
{
	public :
		event_data_base_t( const MSG * message_instance )
			:	m_message_instance( message_instance )
		{}

		//! Access to message.
		const MSG&
		operator * () const
		{
			return *m_message_instance;
		}

		//! Access to raw message pointer.
		const MSG *
		get() const
		{
			return m_message_instance;
		}

		//! Access to message via pointer.
		const MSG *
		operator -> () const
		{
			return get();
		}

	private:
		//! Message.
		const MSG * const m_message_instance;
};

//! Template for message incapsulation.
/*!
	Used to wrap a message which is the source for agent event.

	Usage sample:
	\code
	void
	a_sample_t::evt_smth(
		const so_5::rt::event_data_t< sample_message_t > & msg )
	{
		// ...
	}
	\endcode
*/
template< class MESSAGE >
class event_data_t
	:	public event_data_base_t<
				MESSAGE,
				std::is_base_of< signal_t, MESSAGE >::value >
{
		//! Alias for base type.
		typedef event_data_base_t<
						MESSAGE,
						std::is_base_of< signal_t, MESSAGE >::value >
				base_type_t;

	public:
		//! Constructor.
		event_data_t( const MESSAGE * message_instance )
			:	base_type_t( message_instance )
		{}
};

} /* namespace rt */

} /* namespace so_5 */

#endif

