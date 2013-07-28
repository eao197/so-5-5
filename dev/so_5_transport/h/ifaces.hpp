/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Основные интерфейсы транспортного слоя SObjectizer.
*/

#if !defined( _SO_5_TRANSPORT__IFACES_HPP_ )
#define _SO_5_TRANSPORT__IFACES_HPP_

#include <string>
#include <memory>

#include <ace/Addr.h>

#include <cpp_util_2/h/nocopy.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/ret_code.hpp>
#include <so_5_transport/h/channel_controller.hpp>

namespace so_5_transport
{

/*!
	\brief Тип идентификатора транспортного канала.
 */
typedef unsigned long channel_id_t;

//
// connector_controller_t
//

//! Интерфейс управления объектом-connector-ом.
class SO_5_TRANSPORT_TYPE connector_controller_t
	:
		private cpp_util_2::nocopy_t
{
	public :
		connector_controller_t();
		virtual ~connector_controller_t();

		//! Инициирование операции установления соединения с
		//! удаленной стороной.
		/*!
			Инциирует операции по подключению к серверной точке входа
			на удаленной стороне. Если операция начата успешно,
			то о ее результате владельцу канала будет сообщено
			с помощью сообщения msg_channel_created отправляемого в
			\a ta_mbox.

			Если канал будет успешно создан, то все последующие уведомления
			о состоянии канала будут доставляться владельцу канала с помощью
			\a ta_mbox.
		 */
		virtual so_5::ret_code_t
		connect(
			//! Mbox транспортного агента.
			const so_5::rt::mbox_ref_t & ta_mbox ) = 0;
};

//
// connector_controller_unique_ptr_t
//

//! Псевдоним unique_ptr для connector_controller_t.
typedef std::unique_ptr< connector_controller_t >
	connector_controller_unique_ptr_t;

//
// acceptor_controller_t
//

//! Интерфейс управления объектом-acceptor-ом.
class SO_5_TRANSPORT_TYPE acceptor_controller_t
	:
		private cpp_util_2::nocopy_t
{
	public :
		acceptor_controller_t();
		virtual ~acceptor_controller_t();

		//! Создание серверного канала.
		/*!
			Предполагается, что все необходимые параметры создания серверного
			соединения будут задаваться в конструкторе конкретного
			наследника класса acceptor_controller_t.

			\retval 0 если создание прошло успешно.
		 */
		virtual so_5::ret_code_t
		create(
			//! Mbox транспортного агента.
			const so_5::rt::mbox_ref_t & ta_mbox ) = 0;
};

//
// acceptor_controller_unique_ptr_t
//

//! Псевдоним unique_ptr для acceptor_controller_t.
typedef std::unique_ptr< acceptor_controller_t >
	acceptor_controller_unique_ptr_t;

} /* namespace so_5_transport */

#endif
