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

namespace so_5
{

namespace rt
{

class mbox_t;

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
{
	public:
		//! Default constructor.
		/*!
		 * Used for creating signal events, e.g. events without real
		 * message data.
		 */
		event_data_t()
			:
				m_message_instance( 0 )
		{}

		//! Constructor.
		event_data_t( const MESSAGE * message_instance )
			:
				m_message_instance( message_instance )
		{}

		//! Access to message.
		const MESSAGE&
		operator * () const
		{
			return *m_message_instance;
		}

		//! Access to raw message pointer.
		const MESSAGE *
		get() const
		{
			return m_message_instance;
		}

		//! Access to message via pointer.
		const MESSAGE *
		operator -> () const
		{
			return get();
		}

	private:
		//! Message.
		const MESSAGE * const m_message_instance;
};

//! Template for message incapsulation.
/*!
	Requires presence of message data. Should be used when event requires
	message data and don't allows signals (events without message data).

	Usage sample:
	\code
	void
	a_sample_t::evt_smth(
		const so_5::rt::not_null_event_data_t< sample_message_t > & msg )
	{
		// ...
	}
	\endcode
*/
template< class MESSAGE >
class not_null_event_data_t
{
	public:
		// Constructor.
		not_null_event_data_t( const MESSAGE & message_instance )
			:
				m_message_instance( message_instance )
		{}

		//! Access to message.
		const MESSAGE&
		operator * () const
		{
			return *m_message_instance;
		}

		//! Access to raw message pointer.
		const MESSAGE *
		get() const
		{
			return &m_message_instance;
		}

		//! Access to message via pointer.
		const MESSAGE *
		operator -> () const
		{
			return get();
		}

	private:
		//! Message.
		const MESSAGE & m_message_instance;
};

} /* namespace rt */

} /* namespace so_5 */

#endif

