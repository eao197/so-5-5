/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Реализация acceptor_controller для сокетов.
*/

#if !defined( _SO_5_TRANSPORT__SOCKET__IMPL__ACCEPTOR_CONTROLLER_HPP_ )
#define _SO_5_TRANSPORT__SOCKET__IMPL__ACCEPTOR_CONTROLLER_HPP_

#include <so_5_transport/h/ret_code.hpp>

#include <so_5_transport/socket/h/pub.hpp>

#include <so_5_transport/socket/impl/h/acceptor.hpp>

namespace so_5_transport
{

namespace socket
{

namespace impl
{

//
// socket_acceptor_controller_t
//

/*!
	\brief Реализация acceptor_controller для сокетов.
*/
class SO_5_TRANSPORT_TYPE socket_acceptor_controller_t
	:
		public acceptor_controller_t
{
	private:
		//! Параметры для серверного канала.
		const acceptor_params_t m_params;

		//! Параметры для клиентских каналов.
		const channel_params_t m_channel_params;

		//! Объект-ACCEPTOR, который выполняет прием новых подключений.
		std::unique_ptr< acceptor_t > m_acceptor;

		//! Объект, который отвечает за назначение новым соединениям
		//! нестандартных параметров.

		option_setter_unique_ptr_t m_option_setter;
		//! Реактор, с которым нужно будет работать.
		/*!
		 * Используется для организации вызова m_acceptor.open().
		 */
		ACE_Reactor * m_reactor;

	public :
		//! Инициализирующий конструктор.
		socket_acceptor_controller_t(
			//! Параметры для серверного канала.
			const acceptor_params_t & params,
			//! Параметры для клиентских каналов.
			const channel_params_t & channel_params,
			//! Объект, который отвечает за назначение новым соединениям
			//! нестандартных параметров.
			option_setter_unique_ptr_t option_setter,
			//! Реактор, с которым нужно будет работать.
			ACE_Reactor * reactor );
		virtual ~socket_acceptor_controller_t();

		//! \name Реализация унаследованных методов.
		//! \{
		virtual so_5::ret_code_t
		create(
			const so_5::rt::mbox_ref_t & ta_mbox );
		//! \}
};

} /* namespace impl */

} /* namespace socket */

} /* namespace so_5_transport */

#endif

