/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Набор функции для удобного запуска SObjectizer.

	Для того чтобы запустить SObjectizer Environment не обязательно создавать
	собственный класс-наследник so_5::rt::so_environment_t.
	В состав SObjectizer входит ряд функций, которые упрощают запуск
	SObjectizer Environment.
	Эти функции можно условно разделить на пары:
	версия с параметрами SObjectizer Environment и без.

	В данном файле находятся объявления этих функций.
*/

#if !defined( _SO_5__API__API_HPP_ )
#define _SO_5__API__API_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;
class so_environment_params_t;

} /* namespace rt */


namespace api
{

//! Определение соглажения вызова функций.
#if defined( SO_5__PLATFORM_REQUIRES_CDECL )
	#define SO_5_MODULE_CALLING_CONVENTION __cdecl
#else
	#define SO_5_MODULE_CALLING_CONVENTION
#endif

//! Псевдоним для простой функции инициализации SObjectizer.
typedef
	void
	(SO_5_MODULE_CALLING_CONVENTION *pfn_so_environment_init_t)(
		so_5::rt::so_environment_t & );

//! Запустить среду SO с параметрами.
/*!
	Пример:
	\code
	void
	init( so_5::rt::so_environment_t & env )
	{
		auto coop = env.create_coop( "main_coop" );
		coop->add_agent(
			so_5::rt::agent_ref_t(
				new a_main_t ) );

		env.register_coop( std::move( coop ) );
	}

	...

	int
	main( int argc, char * argv[] )
	{
		return so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
				.mbox_mutex_pool_size( 16 )
				.agent_coop_mutex_pool_size( 16 )
				.agent_event_queue_mutex_pool_size( 16 ),
			so_5::DO_NOT_THROW_ON_ERROR );
	}
	\endcode
*/
SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	//! Указатель на функцию, которая выполняет инициализацию.
	pfn_so_environment_init_t init_func,
	//! Параметры среды.
	const so_5::rt::so_environment_params_t & env_params,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR );

//! Запустить среду SO с параметрами по умолчанию.
/*!
	Пример:
	\code
	void
	init( so_5::rt::so_environment_t & env )
	{
		auto coop = env.create_coop( "main_coop" );
		coop->add_agent(
			so_5::rt::agent_ref_t(
				new a_main_t ) );

		env.register_coop( std::move( coop ) );
	}

	...

	int
	main( int argc, char * argv[] )
	{
		try
		{
			so_5::api::run_so_environment( &init );
		}
		catch( const std::exception & ex )
		{
			std::cerr << "Error: " << ex.what() << std::endl;
			return 1;
		}

		return 0;
	}
	\endcode
*/
SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	//! Указатель на функцию, которая выполняет инициализацию.
	pfn_so_environment_init_t init_func,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR );

//! Базовый класс для вызова инициализирующей функции SObjectizer.
class SO_5_TYPE env_init_caller_base_t
{
	public:

		virtual ~env_init_caller_base_t();

		virtual void
		call( so_5::rt::so_environment_t & ) = 0;
};

//! Запустить среду SO с параметрами по умолчанию.
/*!
	Инициализирует SObjectizer через env_init_caller_base_t.
	Используется для работы run_so_environment_with_parameter() и
	run_so_environment_on_object().
*/
SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	env_init_caller_base_t & init_caller,
	throwing_strategy_t throwing_strategy );

//! Запустить среду SO с параметрами.
/*!
	Инициализирует SObjectizer через env_init_caller_base_t.
*/
SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	env_init_caller_base_t & init_caller,
	const so_5::rt::so_environment_params_t & env_params,
	throwing_strategy_t throwing_strategy );


//! Вызыватель функции инициализации SObjectizer с параметром.
/*!
	Позволяет передать в инициализирующую функцию дополнительный параметр.
	Например:
	\code
		void
		init(
			so_5::rt::so_environment_t & env,
			const std::string & server_addr )
		{
			// Создаем кооперацию
			so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
				so_5::rt::nonempty_name_t( "test_server_application" ),
				so_5::disp::active_obj::create_disp_binder(
					"active_obj" ) );

			so_5_transport::socket::acceptor_controller_creator_t
				acceptor_creator( env );

			using so_5_transport::a_server_transport_agent_t;
			std::unique_ptr< a_server_transport_agent_t > ta(
				new a_server_transport_agent_t(
					env,
					acceptor_creator.create( server_addr ) ) );

			so_5::rt::agent_ref_t serv(
				new a_main_t( env, ta->query_notificator_mbox() ) );

			coop->add_agent( serv );
			coop->add_agent( so_5::rt::agent_ref_t( ta.release() ) );

			// Регистрируем кооперацию
			so_5::ret_code_t rc = env.register_coop( coop );

			// Обработка ошибок и логика завершения приложения.
		}

		// ...

		int
		main( int argc, char ** argv )
		{
			if( 2 == argc )
			{
				std::string server_addr( argv[ 1 ] );

				return so_5::api::run_so_environment_with_parameter(
					&init,
					server_addr,
					so_5::rt::so_environment_params_t()
						.add_named_dispatcher(
							so_5::rt::nonempty_name_t( "active_obj" ),
							so_5::disp::active_obj::create_disp() )
						.add_layer(
							std::unique_ptr< so_5_transport::reactor_layer_t >(
								new so_5_transport::reactor_layer_t ) ) );
			}
			else
				std::cerr << "sample.server <port>" << std::endl;

			return 0;
		}
	\endcode
*/

template< class PARAM_TYPE >
class env_init_with_param_caller_t
	:
		public env_init_caller_base_t
{
	public:
		//! Псевдоним инициализирующей функции с параметром.
		typedef void
		(SO_5_MODULE_CALLING_CONVENTION *INIT_FUNC_T)(
			so_5::rt::so_environment_t &, const PARAM_TYPE & );

		env_init_with_param_caller_t(
			INIT_FUNC_T init_func,
			const PARAM_TYPE & param )
			:
				m_init_func( init_func ),
				m_param( param )
		{}

		virtual ~env_init_with_param_caller_t()
		{}

		virtual void
		call( so_5::rt::so_environment_t & env )
		{
			(*m_init_func)( env, m_param );
		}

	private:
		INIT_FUNC_T m_init_func;
		const PARAM_TYPE m_param;
};

//! Запустить SObjectizer c параметризованной функцией.
template< class PARAM_TYPE >
so_5::ret_code_t
run_so_environment_with_parameter(
	//! Инициализирующая функция с параметром.
	/*!
		Функция должна иметь вид: <i>void init( env, my_param )</i>.
	*/
	typename env_init_with_param_caller_t< PARAM_TYPE >::INIT_FUNC_T
		init_func,
	//! Дополнительный параметр.
	const PARAM_TYPE & param,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
{
	env_init_with_param_caller_t< PARAM_TYPE >
		env_init_caller( init_func, param );

	return run_so_environment( env_init_caller, throwing_strategy );
}

//! Запустить SObjectizer c параметризованной функцией с заданными параметрами.
template< class PARAM_TYPE >
so_5::ret_code_t
run_so_environment_with_parameter(
	//! Инициализирующая функция с параметром.
	/*!
		Функция должна иметь вид: <i>void init( env, my_param )</i>.
	*/
	typename env_init_with_param_caller_t< PARAM_TYPE >::INIT_FUNC_T
		init_func,
	//! Дополнительный параметр.
	const PARAM_TYPE & param,
	//! Параметры SObjectizer.
	const so_5::rt::so_environment_params_t & env_params,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
{
	env_init_with_param_caller_t< PARAM_TYPE >
		env_init_caller( init_func, param );

	return run_so_environment( env_init_caller, env_params, throwing_strategy );
}

//! Вызыватель функции инициализации SObjectizer как метод класса.
/*!
	Позволяет делегировать инициализацию SObjectizer
	методу класса.
	Например:
	\code
		struct client_data_t
		{
			std::string m_server_addr;
			protocol_parser_t m_protocol_parser;

			void
			init( so_5::rt::so_environment_t & env )
			{
				// Создаем кооперацию
				so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
					so_5::rt::nonempty_name_t( "test_client_application" ),
					so_5::disp::active_obj::create_disp_binder(
						"active_obj" ) );

				so_5_transport::socket::connector_controller_creator_t
					connector_creator( env );

				using so_5_transport::a_client_transport_agent_t;
				std::unique_ptr< a_client_transport_agent_t > ta(
					new a_client_transport_agent_t(
						env,
						connector_creator.create( m_server_addr ) ) );

				so_5::rt::agent_ref_t client(
					new a_main_t(
						env,
						ta->query_notificator_mbox(),
						m_rest_of_argv ) );

				coop->add_agent( client );
				coop->add_agent( so_5::rt::agent_ref_t( ta.release() ) );

				// Регистрируем кооперацию
				so_5::ret_code_t rc = env.register_coop( coop );

				// Обработка ошибок и логика завершения приложения.
			}
		};

		// ...

		int
		main( int argc, char ** argv )
		{
			if( 3 == argc )
			{
				client_data_t client_data;
				client_data.m_server_addr = argv[ 1 ];
				client_data.m_protocol_parser = create_protocol( argv[ 2 ] );

				return so_5::api::run_so_environment_on_object(
					client_data,
					&client_data_t::init,
					so_5::rt::so_environment_params_t()
						.add_named_dispatcher(
							so_5::rt::nonempty_name_t( "active_obj" ),
							so_5::disp::active_obj::create_disp() )
						.add_layer(
							std::unique_ptr< so_5_transport::reactor_layer_t >(
								new so_5_transport::reactor_layer_t ) ) );
			}
			else
				std::cerr << "sample.client <port> <protocol_version>" << std::endl;

			return 0;
		}
	\endcode
*/
template< class OBJECT >
class env_init_as_method_caller_t
	:
		public env_init_caller_base_t
{
	public:
		typedef void
		(OBJECT::*INIT_METHOD_T)(
			so_5::rt::so_environment_t & );

		env_init_as_method_caller_t(
			OBJECT & obj,
			INIT_METHOD_T init_func )
			:
				m_obj( obj ),
				m_init_func( init_func )
		{}

		virtual ~env_init_as_method_caller_t()
		{}

		virtual void
		call( so_5::rt::so_environment_t & env )
		{
			(m_obj.*m_init_func)( env );
		}

	private:
		OBJECT m_obj;
		INIT_METHOD_T m_init_func;
};

//! Запустить SObjectizer c функцией-методом класса.
template< class OBJECT >
so_5::ret_code_t
run_so_environment_on_object(
	//! Объект инициализатор.
	OBJECT & obj,
	//! Метод инициализации.
	typename env_init_as_method_caller_t< OBJECT >::INIT_METHOD_T
		init_func,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
{
	env_init_as_method_caller_t< OBJECT >
		env_init_caller( obj, init_func );

	return run_so_environment( env_init_caller, throwing_strategy );
}

//! Запустить SObjectizer c функцией-методом класса с заданными параметрами.
template< class OBJECT >
so_5::ret_code_t
run_so_environment_on_object(
	//! Объект инициализатор.
	OBJECT & obj,
	//! Метод инициализации.
	typename env_init_as_method_caller_t< OBJECT >::INIT_METHOD_T
		init_func,
	//! Параметры SObjectizer.
	const so_5::rt::so_environment_params_t & env_params,
	//! Флаг - бросать ли исключение в случае ошибок.
	throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
{
	env_init_as_method_caller_t< OBJECT >
		env_init_caller( obj, init_func );

	return run_so_environment( env_init_caller, env_params, throwing_strategy );
}

} /* namespace api */

} /* namespace so_5 */

#endif
