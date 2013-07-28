/*
	SObjectizer 5 Transport.
*/
/*!
	\file
	\brief Поддержка транспортного уровня посредством сокетов.
*/


#if !defined( _SO_5_TRANSPORT__SOCKET__PUB_HPP_ )
#define _SO_5_TRANSPORT__SOCKET__PUB_HPP_

#include <string>

#include <ace/INET_Addr.h>
#include <ace/Synch_Options.h>
#include <ace/SOCK_Stream.h>
#include <ace/Reactor.h>

#include <so_5_transport/h/declspec.hpp>

#include <so_5/h/throwing_strategy.hpp>
#include <so_5/rt/h/so_environment.hpp>

#include <so_5_transport/h/reactor_layer.hpp>
#include <so_5_transport/h/ifaces.hpp>
#include <so_5_transport/h/channel_params.hpp>

namespace so_5_transport
{

namespace socket
{

//
// modify_old_format_ip
//

/*!
	Проверяет формат IP адреса и, если он начинается сразу с ':',
	то добавляет в начало 'localhost'. Поскольку раньше коммуникационная
	система SObjectizer-а позволяла использовать адреса вида
	':3000', а ACE_INET_Addr::string_to_addr не поддерживает этот формат,
	то следует преобразовывать подобные адреса к 'localhost:3000'.
*/
SO_5_TRANSPORT_EXPORT_FUNC_SPEC( std::string )
modify_old_format_ip( const std::string & ip );

//
// option_setter_t
//

/*!
	\brief Интерфейс объекта, который будет отвечать за установку
	нестандартных опций для объекта SOCK_Stream.

	Создание объектов SOCK_Stream скрыто от пользователя в реализациях
	connector/acceptor_controller. Для того, чтобы пользователь мог
	изменить установки SOCK_Stream, который используются по умолчанию,
	реализации connector/acceptor_controller предоставляют пользователю
	возможность задать объект-конфигуратор, реализующий интерфейс
	option_setter_t. Это объект задействуется при создании SOCK_Stream.
*/
class SO_5_TRANSPORT_TYPE option_setter_t
{
	public:
		virtual ~option_setter_t();

		/*!
			Этот метод вызывается connector-ом/acceptor-ом для назначения
			SOCK_Stream необходимых разработчику опций.
		*/
		virtual so_5::ret_code_t
		setup( ACE_SOCK_Stream & stream ) = 0;

	private:
		// Защита от копирования.
		option_setter_t(
			const option_setter_t & );
		option_setter_t &
		operator=(
			const option_setter_t & );
};

//
// option_setter_unique_ptr_t
//

//! Псевдоним unique_ptr для option_setter.
typedef std::unique_ptr< option_setter_t > option_setter_unique_ptr_t;

//
// connector_params_t
//

/*!
	\brief Параметры, необходимые connector-у для установления
	нового соединения.

	\note Это инкапсуляция параметров для ACE_Connector::connect.
*/
class SO_5_TRANSPORT_TYPE connector_params_t
{
	public:
		/*!
			Конструктор должен сразу получать значение \a remote_addr.
		*/
		connector_params_t(
			const ACE_INET_Addr & remote_addr );
		~connector_params_t();

		//! \name Getter-ы/Setter-ы.
		//! \{
		const ACE_Synch_Options &
		synch_options() const;

		connector_params_t &
		set_synch_options(
			const ACE_Synch_Options & value );

		int
		reuse_addr() const;

		connector_params_t &
		set_reuse_addr(
			int value );

		int
		flags() const;

		connector_params_t &
		set_flags(
			int value );

		const ACE_INET_Addr &
		remote_addr() const;

		connector_params_t &
		set_remote_addr(
			const ACE_INET_Addr & value );

		const ACE_INET_Addr &
		local_addr() const;

		connector_params_t &
		set_local_addr(
			const ACE_INET_Addr & value );
		//! \}

	private:
		//! Параметры синхронного/асинхронного подключения.
		/*!
			Значение по умолчанию: ACE_Synch_Options::default.
		*/
		ACE_Synch_Options m_synch_options;

		//! Признак необходимости переиспользования локального адреса.
		/*!
			По умочанию: 0 (не переиспользуется).
		*/
		int m_reuse_addr;

		//! Флаги для нового подключения.
		/*!
			По умолчанию: O_RDWR.
		*/
		int m_flags;

		//! Адрес удаленного хоста.
		/*!
			Значения по умолчанию не имеет, должен определяться в конструкторе.
		*/
		ACE_INET_Addr m_remote_addr;

		//! Адрес локального хоста.
		/*!
			Значение по умолчанию: значение, назначаемое конструктором
			ACE_INET_Addr по умолчанию.
		*/
		ACE_INET_Addr m_local_addr;
};

/*!
	\brief Вспомогательный метод для упрощения создания connector_params.

	Пример использования:
	\code
	connector_params( remote_ip ).
		set_synch_options( ACE_Synch_Options::asynch ).
		set_reuse_addr( 1 ).
		set_local_addr( local_ip );
	\endcode
*/
SO_5_TRANSPORT_EXPORT_FUNC_SPEC( connector_params_t )
create_connector_params( const std::string & remote_addr );

/*!
	\brief Вспомогательный метод для упрощения создания connector_params.

	Пример использования:
	\code
	connector_params( remote_ip_str ).
		set_synch_options( ACE_Synch_Options::asynch ).
		set_reuse_addr( 1 ).
		set_local_addr( local_ip );
	\endcode
*/
inline connector_params_t
create_connector_params( const ACE_INET_Addr & remote_addr )
{
	return connector_params_t( remote_addr );
}

//
// acceptor_params_t
//

/*!
	\brief Параметры, необходимые acceptor-у для создания
	серверного сокета.

	\note Это инкапсуляция параметров для ACE_Acceptor::open.
*/
class SO_5_TRANSPORT_TYPE acceptor_params_t
{
	public :
		/*!
			Конструктор должен сразу получать значение \a local_addr.
		 */
		acceptor_params_t(
			const ACE_INET_Addr & local_addr );
		~acceptor_params_t();

		//! \name Getter-ы/Setter-ы.
		//! \{
		const ACE_INET_Addr &
		local_addr() const;

		acceptor_params_t &
		set_local_addr(
			const ACE_INET_Addr & value );

		int
		reuse_addr() const;

		acceptor_params_t &
		set_reuse_addr(
			int value );

		int
		flags() const;

		acceptor_params_t &
		set_flags(
			int value );

		int
		use_select() const;

		acceptor_params_t &
		set_use_select(
			int value );
		//! \}

	private :
		//! Локальный адрес серверного сокета.
		ACE_INET_Addr m_local_addr;

		//! Признак необходимости переиспользования локального адреса.
		/*!
			По умочанию: 1 (переиспользуется).
		 */
		int m_reuse_addr;

		//! Флаги для Acceptor-а.
		/*!
			По умолчанию: ACE_NONBLOCK.
		 */
		int m_flags;

		//! Нужно ли использовать select() для проверки наличия
		//! готовых клиентских подключений.
		/*!
			По умолчанию: 1.
		 */
		int m_use_select;
};

/*!
	\brief Вспомогательный метод для упрощения создания acceptor_params.

	Пример использования:
	\code
	acceptor_params( local_ip ).
		set_reuse_addr( 0 ).
		set_flags( ACE_NONBLOCK );
	\endcode
*/
inline acceptor_params_t
create_acceptor_params( const ACE_INET_Addr & local_addr )
{
	return acceptor_params_t( local_addr );
}

/*!
	\brief Вспомогательный метод для упрощения создания acceptor_params.

	Пример использования:
	\code
	acceptor_params( local_ip_str ).
		set_reuse_addr( 0 ).
		set_use_select( 0 );
	\endcode
*/
SO_5_TRANSPORT_EXPORT_FUNC_SPEC( acceptor_params_t )
create_acceptor_params( const std::string & local_addr );

/*!
	\name Средства для создания acceptor_controller-ов.
	\{
*/

//
// acceptor_controller_creator_t
//

//! Создатель acceptor_controller-ов.
class SO_5_TRANSPORT_TYPE acceptor_controller_creator_t
{
	public:
		acceptor_controller_creator_t(
			//! Среда SO к которой привязываются acceptor_controller-ы.
			so_5::rt::so_environment_t & env );

		~acceptor_controller_creator_t();

		/*!
			Самый простой вариант, получающий в качестве аргумента строку
			с IP-адресом.

			Все остальные параметры имеют значение по умолчанию.

			\note Поддерживает формат ":<PORT>", например, ":3000".
		*/
		acceptor_controller_unique_ptr_t
		create(
			//! Строка c IP-адресом сервера.
			const std::string & ip_address,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать только параметры acceptor_controller-а.

			Остальные параметры имеют значение по умочанию.
		*/
		acceptor_controller_unique_ptr_t
		create(
			const acceptor_params_t & acceptor_params,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать параметры acceptor_controller-а и
			коммуникационного канала.

			Нельзя установить option_setter и указать реактор.
		*/
		acceptor_controller_unique_ptr_t
		create(
			const acceptor_params_t & acceptor_params,
			const channel_params_t & channel_params,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который не позволяет задавать реактор.
		*/
		acceptor_controller_unique_ptr_t
		create(
			const acceptor_params_t & acceptor_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать именованный реактор.

			Если реактора с таким именем не оказалось, то вернется
			acceptor_controller_unique_ptr_t( 0 ).
		*/
		acceptor_controller_unique_ptr_t
		create(
			const acceptor_params_t & acceptor_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			const std::string & reactor_name,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

	private:
		//! Внутренний метод для создания acceptor_controller.
		acceptor_controller_unique_ptr_t
		create(
			const acceptor_params_t & acceptor_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			ACE_Reactor * reactor,
			so_5::throwing_strategy_t throwing_strategy );

		//! Слой реакторов.
		const reactor_layer_t * const m_reactor_layer;
};

/*!
	\}
*/

/*!
	\name Средства для создания connector_controller-ов.
	\{
*/

//
// connector_controller_creator_t
//

//! Создаитель connector_controller-ов.
class SO_5_TRANSPORT_TYPE connector_controller_creator_t
{
	public:
		connector_controller_creator_t(
			//! Среда SO к которой привязываются connector_controller-ы.
			so_5::rt::so_environment_t & env );
		~connector_controller_creator_t();

		/*!
			Самый простой вариант, получающий в качестве аргумента строку
			с IP-адресом.

			Все остальные параметры имеют значение по умолчанию.

			\note Поддерживает формат ":<PORT>", например, ":3000".
		*/
		connector_controller_unique_ptr_t
		create(
			//! Строка c IP-адресом сервера.
			const std::string & ip_address,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать только параметры connector_controller-а.

			Остальные параметры имеют значение по умочанию.
		*/
		connector_controller_unique_ptr_t
		create(
			const connector_params_t & connector_params,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать параметры connector_controller-а и
			коммуникационного канала.

			Нельзя установить option_setter и указать реактор.
		*/
		connector_controller_unique_ptr_t
		create(
			const connector_params_t & connector_params,
			const channel_params_t & channel_params,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который не позволяет задавать реактор.
		*/
		connector_controller_unique_ptr_t
		create(
			const connector_params_t & connector_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

		/*!
			Вариант, который позволяет задать все.
		*/
		connector_controller_unique_ptr_t
		create(
			const connector_params_t & connector_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			const std::string & reactor_name,
			so_5::throwing_strategy_t throwing_strategy = so_5::THROW_ON_ERROR );

	private:
		//! Внутренний метод для создания connector_controller.
		connector_controller_unique_ptr_t
		create(
			const connector_params_t & connector_params,
			const channel_params_t & channel_params,
			option_setter_unique_ptr_t option_setter,
			ACE_Reactor * reactor,
			so_5::throwing_strategy_t throwing_strategy );

		//! Слой реакторов.
		const reactor_layer_t * const m_reactor_layer;
};

/*!
	\}
*/

} /* namespace socket */

} /* namespace so_5_transport */

#endif
