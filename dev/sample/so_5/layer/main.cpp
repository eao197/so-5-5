/*
  Демострация обмена сообщениями.
*/

#include <iostream>
#include <set>

#include <ace/OS.h>
#include <ace/Time_Value.h>

// Загружаем основные заголовочные файлы SObjectizer.
#include <so_5/api/h/api.hpp>
#include <so_5/rt/h/rt.hpp>
#include <so_5/rt/h/so_layer.hpp>

class msg_shutdown
	:
		public so_5::rt::message_t
{
	public:
		msg_shutdown()
		{}
		virtual ~msg_shutdown()
		{}
};

//! Вспомогательный класс-интерфейс для выполнения подписки.
class event_subscriber_t
{
		event_subscriber_t(
			const event_subscriber_t & );
		void
		operator = (
			const event_subscriber_t & );

	public:
		event_subscriber_t(
			const so_5::rt::state_t & state )
		:
			m_state( state )
			{
			}

		virtual ~event_subscriber_t(){}

		//! Выполнить подписку обработчика данного агента на заданное сообщение.
		//! Обработчик, состояние агента и тип сообщения должны определять потомки.
		virtual so_5::ret_code_t
		subscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox ) = 0;

		//! Состояние агента в котором необходимо осуществлять одписку.
		inline const so_5::rt::state_t &
		state() const
		{
			return m_state;
		}

	protected:
		const so_5::rt::state_t & m_state;
};

//! Реализация подписчика для конкретных типов сообщений.
template< class AGENT >
class real_event_subscriber_t
	:
		public event_subscriber_t
{
	public:
		//! Указатель на метод обработчик события агента.
		typedef void (AGENT::*FN_PTR_T)(
			const so_5::rt::event_data_t< msg_shutdown > & );

		real_event_subscriber_t(
			FN_PTR_T pfn,
			const so_5::rt::state_t & state )
			:
				event_subscriber_t( state ),
				m_pfn( pfn )
		{}

		virtual ~real_event_subscriber_t()
		{}

		virtual so_5::ret_code_t
		subscribe(
			//! Агент.
			so_5::rt::agent_t & agent,
			//! Mbox.
			const so_5::rt::mbox_ref_t & mbox )
		{
			so_5::rt::subscription_bind_t subscription_bind( agent, mbox );
			return subscription_bind
				.in( m_state )
					.event(
						m_pfn,
						so_5::DO_NOT_THROW_ON_ERROR );
		}

	private:
		FN_PTR_T m_pfn;
};

class msg_hello_to_all
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_all(
			const std::string & sender,
			const so_5::rt::mbox_ref_t & mbox )
			:
				m_sender( sender ),
				m_mbox( mbox )
		{}

		virtual ~msg_hello_to_all()
		{}

		// Имя отправителя.
		std::string m_sender;
		// Mbox отправителя.
		so_5::rt::mbox_ref_t m_mbox;
};

class msg_hello_to_you
	:
		public so_5::rt::message_t
{
	public:
		msg_hello_to_you(
			const std::string & sender )
			:
				m_sender( sender )
		{}
		virtual ~msg_hello_to_you()
		{}

		// Имя отправителя.
		std::string m_sender;
};

class shutdowner_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:

		virtual ~shutdowner_layer_t()
		{
		}

		//
		// Методы для контроля работы слоя.
		//

		// Запустить слой.
		virtual so_5::ret_code_t
		start()
		{
			m_shutdown_mbox = so_environment().create_local_mbox(
				so_5::rt::nonempty_name_t( "shutdown_mbox" ) );

			std::unique_ptr< msg_shutdown > msg( new msg_shutdown );

			so_environment().single_timer(
				msg,
				m_shutdown_mbox,
				3*1000 );

			return 0;
		}

		// Инициировать завершение выполнения слоя.
		virtual void
		shutdown()
		{
			std::cout << "shutdowner_layer shutdown()" << std::endl;
		}

		//! Ожидание завершения слоя.
		virtual void
		wait()
		{
			std::cout << "shutdowner_layer wait()" << std::endl;
		}
		//! \}

		template< class AGENT >
		void
		subscribe(
			so_5::rt::agent_t* agent_ptr ,
			const so_5::rt::state_t & agent_state ,
			void (AGENT::*FN_PTR_T)
				(const so_5::rt::event_data_t< msg_shutdown > &) )
		{
			real_event_subscriber_t< AGENT > real_event_subscriber(
				FN_PTR_T,
				agent_state );

			so_5::ret_code_t rc = real_subscribe(
				agent_ptr , real_event_subscriber );

			if( !rc )
				m_subscribers.insert(agent_ptr);
		}

		void
		unsubscribe( so_5::rt::agent_t* agent_ptr )
		{
			std::set<so_5::rt::agent_t*>::iterator it =
				m_subscribers.find(agent_ptr);

			if( it != m_subscribers.end() )

				m_subscribers.erase(it);

				if( m_subscribers.empty() )
				{
					std::cout << "all agents are unsubscribed\n";
					so_environment().stop();
				}
		}

	private:

		so_5::ret_code_t
		real_subscribe(
			so_5::rt::agent_t* agent_ptr ,
			event_subscriber_t & event_subscriber )
		{
			return event_subscriber.subscribe(
				*agent_ptr , m_shutdown_mbox );
		}

		// mbox для оповещения о завершении работы.
		so_5::rt::mbox_ref_t m_shutdown_mbox;

		std::set< so_5::rt::agent_t* > m_subscribers;
};



// C++ описание класса агента.
class a_hello_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public:
		a_hello_t(
			so_5::rt::so_environment_t & env,
			const std::string & agent_name )
			:
				base_type_t( env ),
				m_agent_name( agent_name ),
				m_self_mbox( so_environment().create_local_mbox() ),
				m_common_mbox( so_environment().create_local_mbox(
					so_5::rt::nonempty_name_t( "common_mbox" ) ) )
		{}
		virtual ~a_hello_t()
		{}

		// Определение агента.
		virtual void
		so_define_agent();

		// Обработка начала работы агента в системе.
		virtual void
		so_evt_start();

		void
		evt_hello_to_all(
			const so_5::rt::event_data_t< msg_hello_to_all > & evt_data );

		void
		evt_hello_to_you(
			const so_5::rt::event_data_t< msg_hello_to_you > & evt_data );

		void
		evt_shutdown(
			const so_5::rt::event_data_t< msg_shutdown > & evt_data );

	private:
		// Имя агента.
		const std::string m_agent_name;

		// Mbox данного агента.
		so_5::rt::mbox_ref_t m_self_mbox;

		// Общий mbox для всех агентов.
		so_5::rt::mbox_ref_t m_common_mbox;

};

void
a_hello_t::so_define_agent()
{
	// Подписываемся на сообщения.
	so_subscribe( m_common_mbox )
		.event( &a_hello_t::evt_hello_to_all );

	so_subscribe( m_self_mbox )
		.event( &a_hello_t::evt_hello_to_you );

	so_environment().query_layer< shutdowner_layer_t >()->subscribe(
		this ,
		so_default_state() ,
		&a_hello_t::evt_shutdown );
}

void
a_hello_t::so_evt_start()
{
	std::cout << m_agent_name << ".so_evt_start" << std::endl;

	// Отсылаем всем приветствие.
	m_common_mbox->deliver_message( std::unique_ptr< msg_hello_to_all >(
		new msg_hello_to_all(
			m_agent_name,
			m_self_mbox ) ) );
}

void
a_hello_t::evt_hello_to_all(
	const so_5::rt::event_data_t< msg_hello_to_all > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_all: "
		<< evt_data->m_sender << std::endl;

	// Если привествие слали не мы, то отошлем ответ.
	if( m_agent_name != evt_data->m_sender )
	{
		so_5::rt::mbox_ref_t mbox = evt_data->m_mbox;
		mbox->deliver_message( std::unique_ptr< msg_hello_to_you >(
			new msg_hello_to_you( m_agent_name ) ) );
	}
}

void
a_hello_t::evt_hello_to_you(
	const so_5::rt::event_data_t< msg_hello_to_you > & evt_data )
{
	std::cout << m_agent_name << ".evt_hello_to_you: "
		<< evt_data->m_sender << std::endl;
}

void
a_hello_t::evt_shutdown(
	const so_5::rt::event_data_t< msg_shutdown > & evt_data )
{
	std::cout << m_agent_name << " : preparing to shutdown\n";
	so_environment().query_layer< shutdowner_layer_t >()->unsubscribe(
		this );
}

void
init( so_5::rt::so_environment_t & env )
{
	// Создаем кооперацию
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
		so_5::rt::nonempty_name_t( "coop" ) );

	// Добавляем в кооперацию агентов.
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "alpha" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "beta" ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new a_hello_t( env, "gamma" ) ) );

	// Регистрируем кооперацию.
	env.register_coop( std::move( coop ) );

	// Даем время агентам обменяться сообщениями.
	ACE_OS::sleep( ACE_Time_Value( 0, 200*1000 ) );
	//env.stop();
}

int
main( int, char ** )
{
	try
	{
		/*
		shutdowner_layer_t * ptr = new shutdowner_layer_t();
		so_5::rt::so_environment_params_t so_params;
		so_params.add_layer(
			 std::unique_ptr<shutdowner_layer_t>( ptr ) );
		so_5::api::run_so_environment( &init , so_params );
		*/

		so_5::api::run_so_environment( &init , so_5::rt::so_environment_params_t()
			.add_layer( std::unique_ptr< shutdowner_layer_t > ( new shutdowner_layer_t() ) ) ) ;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
