/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Базовый класс агента выполняющего роль клиента.

	Определяет обработчики событий канала, подписывает агента на события
	и делегирует обработку hook-методам.
*/

#if !defined( _SO_5_TRANSPORT__A_CLIENT_BASE_HPP_ )
#define _SO_5_TRANSPORT__A_CLIENT_BASE_HPP_

#include <set>

#include <so_5/rt/h/agent.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/messages.hpp>
#include <so_5_transport/h/a_channel_base.hpp>

namespace so_5_transport
{

//
// a_client_base_t
//

//! Базовый класс агента выполняющего роль клиента.
class SO_5_TRANSPORT_TYPE a_client_base_t
	:
		public a_channel_base_t
{
		typedef a_channel_base_t base_type_t;

	public:
		a_client_base_t(
			so_5::rt::so_environment_t & env,
			//! Mbox на который будут приходить уведомления о событиях канала.
			const so_5::rt::mbox_ref_t & notification_mbox );
		virtual ~a_client_base_t();
};

} /* namespace so_5_transport */

#endif
