/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Интерфейс контроллера транспортного канала.
*/

#if !defined( _SO_5_TRANSPORT__CHANNEL_CONTROLLER_HPP_ )
#define _SO_5_TRANSPORT__CHANNEL_CONTROLLER_HPP_

#include <ace/Addr.h>

#include <cpp_util_2/h/nocopy.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/ret_code.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/mbox_ref.hpp>

namespace so_5_transport
{

//
// channel_controller_t
//

/*!
	\brief Интерфейс для контроллера канала.
*/
class SO_5_TRANSPORT_TYPE channel_controller_t
	:
		private so_5::rt::atomic_refcounted_t
{
		friend class channel_controller_ref_t;
	public:
		channel_controller_t();
		virtual ~channel_controller_t();

		//! Получение адреса на локальной стороне в виде строки.
		virtual const std::string &
		local_address_as_string() const = 0;

		//! Получение адреса на удаленной строне в виде строки.
		virtual const std::string &
		remote_address_as_string() const = 0;

		//! Инициирование контроля за готовностью канала к чтению.
		/*!
			Инициирует операции по проверке канала на наличие входящих
			данных. Если операция начата успешно, то при обнаружении
			входящих данных в указанный mbox будет отправляться
			сообщение msg_incoming_package.
		 */
		virtual so_5::ret_code_t
		enforce_input_detection() = 0;

		//! Инициировать закрытие канала,
		//! После вызова этого метода, все действия
		//! с каналом должны игнорироваться.
		virtual so_5::ret_code_t
		close() = 0;
};

//
// channel_controller_ref_t
//


//! Умная ссылка на channel_controller_t.
class SO_5_TRANSPORT_TYPE channel_controller_ref_t
{
	public:
		channel_controller_ref_t();

		explicit channel_controller_ref_t(
			channel_controller_t * channel_controller );

		channel_controller_ref_t(
			const channel_controller_ref_t & channel_controller_ref );

		void
		operator = ( const channel_controller_ref_t & channel_controller_ref );

		~channel_controller_ref_t();

		inline const channel_controller_t *
		get() const
		{
			return m_channel_controller_ptr;
		}

		inline channel_controller_t *
		get()
		{
			return m_channel_controller_ptr;
		}

		inline const channel_controller_t *
		operator -> () const
		{
			return m_channel_controller_ptr;
		}

		inline channel_controller_t *
		operator -> ()
		{
			return m_channel_controller_ptr;
		}

		inline channel_controller_t &
		operator * ()
		{
			return *m_channel_controller_ptr;
		}


		inline const channel_controller_t &
		operator * () const
		{
			return *m_channel_controller_ptr;
		}

		inline bool
		operator == ( const channel_controller_ref_t & channel_controller_ref ) const
		{
			return m_channel_controller_ptr ==
				channel_controller_ref.m_channel_controller_ptr;
		}

		inline bool
		operator < ( const channel_controller_ref_t & channel_controller_ref ) const
		{
			return m_channel_controller_ptr <
				channel_controller_ref.m_channel_controller_ptr;
		}

	private:
		//! Увеличить количество ссылок на channel_controller
		//! и в случае необходимости удалить его.
		void
		inc_channel_controller_ref_count();

		//! Уменьшить количество ссылок на channel_controller
		//! и в случае необходимости удалить его.
		void
		dec_channel_controller_ref_count();

		//! Подконтрольный channel_controller_t.
		channel_controller_t * m_channel_controller_ptr;

};

} /* namespace so_5_transport */

#endif
