/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Реализация Svc_Handler.
*/

#include <cassert>

#include <so_5_transport/socket/impl/h/channel_controller_impl.hpp>

#include <so_5_transport/socket/impl/h/svc_handler.hpp>

#define SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT(s) "[%D PID:%P TID:%t] %M -- " s " @%N:%l\n"

namespace so_5_transport
{

namespace socket
{

namespace impl
{

namespace /* ananymous */
{

//
// output_flush_notifier_impl_t
//

class output_flush_notifier_impl_t
	:
		public block_stream_notifiers::flush_notifier_t
{
		output_flush_notifier_impl_t( const output_flush_notifier_impl_t & );
		void
		operator = ( const output_flush_notifier_impl_t & );

		friend class so_5_transport::socket::impl::svc_handler_t;

		output_flush_notifier_impl_t(
			svc_handler_t & svc_handler )
			:
				m_do_notify( true ),
				m_svc_handler( svc_handler )
		{}

	public:
		virtual ~output_flush_notifier_impl_t()
		{}

		virtual void
		notify()
		{
			if( m_do_notify )
			{
				m_svc_handler.output_notify();
				m_do_notify = false;
			}
		}

		virtual void
		reset()
		{
			m_do_notify = true;
		}

	private:
		bool m_do_notify;
		svc_handler_t & m_svc_handler;
};

class input_free_blocks_availability_notifier_impl_t
	:
		public block_stream_notifiers::free_blocks_availability_notifier_t
{
		input_free_blocks_availability_notifier_impl_t(
			const input_free_blocks_availability_notifier_impl_t & );
		void
		operator = ( const input_free_blocks_availability_notifier_impl_t & );

		friend class so_5_transport::socket::impl::svc_handler_t;

		input_free_blocks_availability_notifier_impl_t(
			svc_handler_t & svc_handler )
			:
				m_input_is_blocked( false ),
				m_svc_handler( svc_handler )
		{}

	public:
		virtual ~input_free_blocks_availability_notifier_impl_t()
		{}

		virtual void
		notify_lacking_free_blocks()
		{
			if( !m_input_is_blocked )
			{
				m_input_is_blocked = true;
				m_svc_handler.block_input();
			}
		}

		virtual void
		notify_free_blocks(
			size_t default_block_count,
			size_t available_block_count )
		{
			if( m_input_is_blocked &&
				available_block_count >= default_block_count )
			{
				m_input_is_blocked = false;
				m_svc_handler.unblock_input();
			}
		}


	private:
		bool m_input_is_blocked;
		svc_handler_t & m_svc_handler;
};

class stream_operation_notifier_impl_t
	:
		public block_stream_notifiers::stream_operation_notifier_t
{
		stream_operation_notifier_impl_t(
			const stream_operation_notifier_impl_t & );
		void
		operator = ( const stream_operation_notifier_impl_t & );

		friend class so_5_transport::socket::impl::svc_handler_t;

		stream_operation_notifier_impl_t(
			const so_5::rt::mbox_ref_t & mbox,
			const msg_incoming_package & msg_template )
			:
				m_do_send_notification( true ),
				m_mbox( mbox ),
				m_msg_template( msg_template )
		{}

	public:
		virtual ~stream_operation_notifier_impl_t()
		{}

		virtual void
		notify_blocks_pushed_back()
		{
			if( m_do_send_notification )
			{
				m_do_send_notification = false;
				m_mbox->deliver_message(
					std::unique_ptr< msg_incoming_package >(
						new msg_incoming_package( m_msg_template ) ) );
			}
		}

		virtual void
		notify_blocks_pushed_front()
		{}

		virtual void
		notify_blocks_queried()
		{
			m_do_send_notification = true;
		}

	private:
		bool m_do_send_notification;
		so_5::rt::mbox_ref_t m_mbox;
		const msg_incoming_package m_msg_template;
};

std::string
addr_to_string( const ACE_INET_Addr & addr )
{
	char text[ BUFSIZ ];
	addr.addr_to_string( text, BUFSIZ );
	return text;
}

} /* ananymous namespace */


//
// last_problem_t
//

const char *
last_problem_t::operator () (
	const std::string & problem )
{
	m_last_problem = problem;
	return m_last_problem.c_str();
}

const std::string &
last_problem_t::str() const
{
	return m_last_problem;
}

//
// svc_handler_t
//
svc_handler_t::~svc_handler_t()
{
}

svc_handler_t::svc_handler_t()
	:
		base_type_t( nullptr, nullptr, nullptr ),
		m_channel_id( 0 )
{
}

svc_handler_t::svc_handler_t(
	ACE_Reactor * reactor,
	channel_id_t channel_id,
	const so_5::rt::mbox_ref_t & ta_mbox,
	const channel_params_t & channel_params )
	:
		base_type_t( nullptr, nullptr, reactor ),
		m_channel_id( channel_id ),
		m_ta_mbox( ta_mbox ),
		m_channel_params( channel_params ),
		m_timer_id( -1 ),
		m_enforce_input_detection( false ),
		m_is_input_blocked( true ),
		m_is_output_blocked( false )
{
}

int
svc_handler_t::open( void * )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::open" );

	assert( nullptr != this->reactor() );

	int result =
		this->reactor()->register_handler(
			this,
			ACE_Event_Handler::EXCEPT_MASK );

	if( 0 == result )
	{
		ACE_INET_Addr local_addr;
		result = this->peer().get_local_addr( local_addr );
		if( 0 != result )
		{
			ACE_ERROR_RETURN( ( LM_ERROR,
				SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
				m_last_problem(
					"unable to get local address" ) ),
				result );
		}
		ACE_INET_Addr remote_addr;
		result = this->peer().get_remote_addr( remote_addr );

		if( 0 != result )
			ACE_ERROR_RETURN( ( LM_ERROR,
				SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
				m_last_problem(
					"unable to get remote address" ) ),
				result );

		m_timer_id =
			this->reactor()->schedule_timer(
				this,
				0,
				ACE_Time_Value( m_channel_params.time_checking_period(), 0 ),
				ACE_Time_Value( m_channel_params.time_checking_period(), 0 ) );

		if( -1 != m_timer_id )
		{
			m_controller = channel_controller_ref_t(
				new channel_controller_impl_t(
					addr_to_string( local_addr ),
					addr_to_string( remote_addr ),
					this ) );

			create_streams();

			m_ta_mbox->deliver_message(
				std::unique_ptr< msg_channel_created >(
					new msg_channel_created(
						m_channel_id,
						channel_io_t( m_input_stream, m_output_stream ),
						m_controller ) ) );

			result = 0;
		}
		else
		{
			result = -1;
			m_ta_mbox->deliver_message(
				std::unique_ptr< msg_channel_failed >(
					new msg_channel_failed(
						"schedule timer in reactor fails" ) ) );
		}
	}
	else
		m_ta_mbox->deliver_message(
			std::unique_ptr< msg_channel_failed >(
				new msg_channel_failed( "register handler in reactor fails, "
					"rc = " + cpp_util_2::slexcast( result ) ) ) );

	return result;
}

int
svc_handler_t::handle_close(
	ACE_HANDLE handle,
	ACE_Reactor_Mask mask )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::handle_close" );

	int result = 0;

	if( nullptr != m_output_stream.get() )
		m_output_stream->reset_flush_notifier();
	if( nullptr != m_input_stream.get() )
		m_input_stream->reset_free_blocks_availability_notifier();
	if( nullptr != m_stream_operation_notifier.get() )
		m_input_stream->reset_stream_operation_notifier();

	m_output_flush_notifier.reset();
	m_input_free_blocks_availability_notifier.reset();
	m_stream_operation_notifier.reset();

	if( nullptr != m_controller.get() )
	{
		channel_controller_impl_t * const channel_controller =
			dynamic_cast< channel_controller_impl_t * >(
				m_controller.get() );

		channel_controller->remove_svc_handler_reference();

		m_ta_mbox->deliver_message(
			std::unique_ptr< msg_channel_lost >(
				new msg_channel_lost(
					m_channel_id,
					m_last_problem.str(),
					m_controller ) ) );

		m_controller = channel_controller_ref_t();
	}

	if( -1 != m_timer_id )
		this->reactor()->cancel_timer( m_timer_id );

	this->reactor()->purge_pending_notifications( this );

	return base_type_t::handle_close( handle, mask );
}

int
svc_handler_t::handle_input( ACE_HANDLE )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::handle_input" );

	bool channel_closed_on_remote_host = false;

	const int read_res = read_channel(
		channel_closed_on_remote_host );

	if( -1 == read_res )
		ACE_ERROR_RETURN( ( LM_ERROR,
			SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
			m_last_problem(
				"[handle_input] read_channel failed; errno: " +
				cpp_util_2::slexcast( ACE_OS::last_error() ) ) ),
			-1 );

	if( channel_closed_on_remote_host )
		ACE_ERROR_RETURN( ( LM_INFO,
			SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
			m_last_problem(
				"channel has been closed on remote host" ) ),
			-1 );

	return 0;
}

int
svc_handler_t::handle_output( ACE_HANDLE )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::handle_output" );

	// Выполняем отправку.
	return write_channel();
}

int
svc_handler_t::handle_timeout(
	const ACE_Time_Value & current_time,
	const void * )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::handle_timeout" );

	if( m_enforce_input_detection &&
		m_is_input_blocked &&
		m_io_timestamps.since_last_input().msec() >
		m_channel_params.max_input_block_timeout() * 1000 )
		ACE_ERROR_RETURN( ( LM_ERROR,
			SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
			m_last_problem(
				"input is in blocked state too long; "
				" timeout: " + cpp_util_2::slexcast(
					m_channel_params.max_input_block_timeout() ) ) ),
			-1 );

	if( m_is_output_blocked )
	{
		const int rc = write_channel();
		switch( rc )
		{
			case 1:
				output_notify();
			case 0:
				break;
			default:
				return rc;
		}

		if( m_is_output_blocked &&
			m_io_timestamps.since_last_output().msec() >
			m_channel_params.max_output_block_timeout() * 1000 )
			ACE_ERROR_RETURN( ( LM_ERROR,
				SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
				m_last_problem(
					"output is in blocked state too long; "
					" timeout: " + cpp_util_2::slexcast(
						m_channel_params.max_output_block_timeout() ) ) ),
				-1 );
	}

	return 0;
}

int
svc_handler_t::handle_exception( ACE_HANDLE fd )
{
	ACE_TRACE( "so_5_transport::impl::socket"
			"::svc_handler_t::handle_exception" );

	return -1;
}

void
svc_handler_t::output_notify()
{
	this->reactor()->notify(
		this,
		ACE_Event_Handler::WRITE_MASK );
}

void
svc_handler_t::block_input()
{
	m_is_input_blocked = true;
	this->reactor()->cancel_wakeup(
		this,
		ACE_Event_Handler::READ_MASK );
}

void
svc_handler_t::unblock_input()
{
	m_is_input_blocked = false;

	this->reactor()->schedule_wakeup(
		this,
		ACE_Event_Handler::READ_MASK );
}

void
svc_handler_t::enforce_input_detection()
{
	m_enforce_input_detection = true;
	unblock_input();
}

int
svc_handler_t::initiate_except()
{
	ACE_Time_Value timeout = ACE_Time_Value::zero;
	return this->reactor()->notify(
		this,
		ACE_Event_Handler::EXCEPT_MASK,
		&timeout );
}

void
svc_handler_t::create_streams()
{
	m_output_stream = scattered_block_stream_ref_t(
		new scattered_block_stream_t(
			m_channel_params.channel_output_stream_params() ) );

	m_input_stream = scattered_block_stream_ref_t(
		new scattered_block_stream_t(
			m_channel_params.channel_input_stream_params() ) );

	m_output_flush_notifier.reset(
		new  output_flush_notifier_impl_t( *this ) );

	m_input_free_blocks_availability_notifier.reset(
		new input_free_blocks_availability_notifier_impl_t( *this ) );

	m_stream_operation_notifier.reset(
		new stream_operation_notifier_impl_t(
			m_ta_mbox,
			msg_incoming_package(
				m_channel_id,
				channel_io_t(
					m_input_stream,
					m_output_stream ) ) ) );

	m_output_stream->reset_flush_notifier(
		m_output_flush_notifier.get() );

	m_input_stream->reset_free_blocks_availability_notifier(
		m_input_free_blocks_availability_notifier.get() );

	m_input_stream->reset_stream_operation_notifier(
		m_stream_operation_notifier.get() );
}


int
svc_handler_t::read_channel(
	bool & channel_closed_on_remote_host )
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::read_channel" );

	raw_block_t * const current_to_read_block = m_input_stream->occupy();

	// Натификатор должен был снять обработчик чтения данных.
	if( nullptr == current_to_read_block )
		return 0;

	ssize_t bytes_read =
		this->peer().recv(
			current_to_read_block->current_write_ptr(),
			current_to_read_block->write_size_available() );

	if( -1 == bytes_read )
		return -1;
	else if( 0 == bytes_read )
		channel_closed_on_remote_host = true;
	else
	{
		current_to_read_block->shift_write_pos( bytes_read );

		m_input_stream->stream_push_back( current_to_read_block );

		m_io_timestamps.mark_last_input();
	}

	return 0;
}

int
svc_handler_t::write_channel()
{
	ACE_TRACE( "so_5_transport::impl::socket"
		"::svc_handler_t::write_channel" );

	raw_block_chain_t chain;

	const size_t bytes_total =
		m_output_stream->stream_query_n_blocks( ACE_IOV_MAX, chain );

	const size_t count = chain.size();

	if( count > 0 && bytes_total > 0 )
	{
		iovec iov[ ACE_IOV_MAX ];
		raw_block_chain_t::iterator it = chain.begin();

		for( size_t i = 0; i < count; ++i, ++it )
		{
			raw_block_t * rb = *it;
			iov[ i ].iov_base =
				const_cast< char *>( rb->current_read_ptr() );
			iov[ i ].iov_len = rb->read_size_available();
		}

		const ssize_t bytes_sent = this->peer().sendv_n( iov, count );

		if( bytes_sent <= 0 )
		{
			m_output_stream->stream_push_front( chain, bytes_total );

			if( EWOULDBLOCK == ACE_OS::last_error() )
			{
				m_is_output_blocked = true;
				ACE_ERROR_RETURN(( LM_ERROR,
					SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
					m_last_problem(
						"sendv_n() blocked; errno: " +
							cpp_util_2::slexcast(
								ACE_OS::last_error() ) ) ),
					0 );
			}
			else
				ACE_ERROR_RETURN(( LM_ERROR,
					SO_5_TRANSPORT_SVC_HANDLER_ERROR_FMT( "%s" ),
					m_last_problem(
						"sendv_n() failed; errno: " +
							cpp_util_2::slexcast(
								ACE_OS::last_error() ) ) ),
					-1 );
		}
		else
		{
			m_is_output_blocked = false;
			m_io_timestamps.mark_last_output();
		}
	}

	// Вернем блоки в буфер потока.
	if( !chain.empty() )
		m_output_stream->release( chain );

	// Если блоки еще есть, то планируем очередную запись.
	if( m_output_stream->check_stream_has_blocks() )
		output_notify();

	return 0;
}

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */
