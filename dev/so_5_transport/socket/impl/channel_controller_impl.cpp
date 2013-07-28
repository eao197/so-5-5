/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/ret_code.hpp>

#include <so_5_transport/socket/impl/h/channel_controller_impl.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

channel_controller_impl_t::channel_controller_impl_t(
	const std::string & local_addr,
	const std::string & remote_addr,
	svc_handler_t * svc_handler )
	:
		m_local_addr( local_addr ),
		m_remote_addr( remote_addr ),
		m_svc_handler( svc_handler ),
		m_enforce_input_detection_called( false )
{
}

const std::string &
channel_controller_impl_t::local_address_as_string() const
{
	return m_local_addr;
}

const std::string &
channel_controller_impl_t::remote_address_as_string() const
{
	return m_remote_addr;
}

so_5::ret_code_t
channel_controller_impl_t::enforce_input_detection()
{
	if( !m_enforce_input_detection_called &&
		nullptr != m_svc_handler )
	{
		scoped_lock_t lock( m_lock );

			if( nullptr != m_svc_handler )
			{
				m_svc_handler->enforce_input_detection();
				m_enforce_input_detection_called = true;
				return 0;
			}
	}

	return rc_controller_is_closed;
}

so_5::ret_code_t
channel_controller_impl_t::close()
{
	// Double-locking
	if( nullptr != m_svc_handler )
	{
		while( true )
		{
			scoped_lock_t lock( m_lock );
			if( nullptr != m_svc_handler )
			{
				if ( 0 == m_svc_handler->initiate_except() )
				{
					m_svc_handler = nullptr;
					return 0;
				}
			}
			else
				break;
		}
	}
	return rc_controller_is_closed;
}

void
channel_controller_impl_t::remove_svc_handler_reference()
{
	scoped_lock_t lock( m_lock );

	m_svc_handler = nullptr;
}

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */
