/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Контроллер канала.
*/

#if !defined( _SO_5_TRANSPORT__SOCKET__IMPL__CHANNEL_CONTROLLER_IMPL_HPP_ )
#define _SO_5_TRANSPORT__SOCKET__IMPL__CHANNEL_CONTROLLER_IMPL_HPP_

#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>

#include <so_5_transport/h/channel_controller.hpp>

#include <so_5_transport/socket/impl/h/svc_handler.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// channel_controller_impl_t
//

//! Контроллер канала.
class channel_controller_impl_t
	:
		public channel_controller_t
{
	public:
		channel_controller_impl_t(
			const std::string & local_addr,
			const std::string & remote_addr,
			svc_handler_t * m_svc_handler );

		//! Реализация унаследованных методов.
		//! \{
		virtual const std::string &
		local_address_as_string() const;

		virtual const std::string &
		remote_address_as_string() const;

		virtual so_5::ret_code_t
		enforce_input_detection();

		virtual so_5::ret_code_t
		close();
		//! \}

		//! Убрать из контроллера инфрмацию
		//! об обработчике.
		void
		remove_svc_handler_reference();

	private:
		const std::string m_local_addr;
		const std::string m_remote_addr;
		//! Псевдоним для удобного использования ACE_Guard.
		typedef ACE_Guard< ACE_Thread_Mutex > scoped_lock_t;

		//! Последнее время чтения.
		ACE_Thread_Mutex m_lock;

		//! Указатель на ace-обработчик канала.
		svc_handler_t * m_svc_handler;

		//! Вызывался ли enforce_input_detection().
		bool m_enforce_input_detection_called;
};

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

#endif
