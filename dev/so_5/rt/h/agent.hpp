/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Базовый класс для агентов.
*/

#if !defined( _SO_5__RT__AGENT_HPP_ )
#define _SO_5__RT__AGENT_HPP_

#include <memory>
#include <map>

#include <so_5/h/declspec.hpp>
#include <so_5/h/types.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>
#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/subscription_bind.hpp>
#include <so_5/rt/h/subscription_key.hpp>
#include <so_5/rt/h/event_caller_block_ref.hpp>
#include <so_5/rt/h/agent_state_listener.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

// Объявления необходимые для работы агента.
class local_event_queue_t;
class message_consumer_link_t;
class so_environment_impl_t;
class state_listener_controller_t;

} /* namespace impl */

class state_t;
class so_environment_t;
class agent_coop_t;

//
// agent_t
//

//! Базовый класс агентов.
/*!
	Либой агент в SObjectizer-е должен быть производным
	от agent_t.

	Базовый класс агента предоставляет различные методы, которые можно выделить
	в следующие основные группы:
	\li методы для взаимодействия с SObjectizer;
	\li предопределенные хук-методы, вызываемые в процессе регистрации кооперации,
	начала и завершения работы агента;
	\li методы подписки на сообщения и отмены подписок;
	\li методы работы с состояниями агента.

	<b>Методы для взаимодействия с SObjectizer.</b>

	Метод so_5::rt::agent_t::so_environment() служит для получения доступа к
	SObjectizer Environment, а следовательно и ко всем методам
	SObjectizer Environment.
	Обращаться к методу можно сразу после создания агента,
	потому что агент привязывается к SObjectizer Environment
	в конструкторе базового класса so_5::rt::agent_t.

	<b>Хук-методы.</b>

	В базовом классе агента определены хук-методы для
	обработки различных ситуаций, которые по умолчанию ничего не делают.

	Метод определения агента -- agent_t::so_define_agent(),
	который вызывается до того как агент начнет работать в SO,
	т.е. до того, как начнется его работа на рабочей нити.
	Вызов этого метода осуществляется в процессе
	регистрации агента. Служит для начальной подписки агента на сообщения.

	Во время жизненного цикла агента можно выделить два события: начала работы
	агента (когда кооперация успешно зарегистрирована и все агенты привязаны
	к диспетчерам) и завершения работы агента, когда кооперация
	дерегистрируется и агент завершил обработку последней заявки
	на обработку сообщения. Для обработки начала работы агента и завершения
	работы агента служат следующие хук-методы:
	agent_t::so_evt_start() и agent_t::so_evt_finish().
	О вызове этих методов заботится SObjectizer,
	а пользователю остается только определить логику работы этих методов.

	<b>Методы подписки на сообщения и отмены подписок.</b>

	Механизм обработчиков сообщений строится на понятиях сообщения, mbox-а
	(ящик сообщений), обработчика сообщения (метода агента с особой сигнатурой)
	и на понятии состояния агента.

	Обработчиком сообщения может считаться любой метод агента с сигнатурой
	аналогичной следующей:
	\code
		void
		evt_handler(
			const so_5::rt::event_data_t< MESSAGE > & msg );
	\endcode
	Где \c evt_handler — имя обработчика события, а \c MESSAGE — тип сообщения.

	Класс so_5::rt::event_data_t является оберткой над указателем
	на объект типа \c MESSAGE, который предоставляет доступ к объекту
	аналогично <tt>std::unique_ptr</tt>. Причем самого объекта
	может и не существовать, когда отправитель сообщения не имеет
	намерения отправлять в сообщении никакой дополнительной информации,
	кроме типа сообщения, в таком случае сообщение представляет собой
	сигнал определенного типа и ничего более.

	Подписка агента на сообщения осуществляется с помощью метода
	so_5::rt::agent_t::so_subscribe().
	В результате создается объект класса
	so_5::rt::subscription_bind_t, который уже
	знает агента подписчика и mbox, к которому происходит подписка,
	а также в качестве состояния подписки выбрано
	состояние агента по умолчанию.

	<b>Методы работы с состояниями агента</b>

	Для смены состояния агента служит метод
	so_5::rt::agent_t::so_change_state().
	Ошибка, при смене состояния, может возникнуть только тогда,
	когда производится попытка перевести агента в состояние,
	которым он не владеет. В некоторых случаях нужно определять моменты
	смены состояния агента.
	Например, при создании средств мониторинга приложения.
	Для этих случаев предназначены "слушатели"состояния агента.
	Агенту может быть назначено любое количество "слушателей".
	Для добавления слушателя, владение которым контролирует прикладной
	программист, служит метод
	so_5::rt::agent_t::so_add_nondestroyable_listener().
	Для добавления слушателя, владение которым передается агенту,
	служит метод so_5::rt::sgent_t::so_add_destroyable_listener().
*/
class SO_5_TYPE agent_t
	:
		private atomic_refcounted_t
{
		friend class subscription_bind_t;
		friend class subscription_unbind_t;
		friend class agent_ref_t;
		friend class agent_coop_t;

	public:
		//! Конструктор агента.
		/*!
			При создании объекта-агента, он в обязательном
			порядке должен быть привязан к SObjectizer Environment,
			и эта привязка не может быть изменена в дальнейшем.
		*/
		explicit agent_t(
			//! Среда SO, для которой создается агент.
			so_environment_t & env );

		virtual ~agent_t();

		//! Получить указатель на себя.
		/*!
			Для того, чтобы избежать многословных предупреждений
			от компилятора, когда в списке инициализации
			требуется испобльзовать this. Например, при инициализации
			состояний агента.
			\code
				class a_sample_t
					:
						public so_5::rt::agent_t
				{
						typedef so_5::rt::agent_t base_type_t;

						// Состояние агента.
						const so_5::rt::state_t m_sample_state;
					public:
						a_sample_t( so_5::rt::so_environment_t & env )
							:
								base_type_t( env ),
								m_sample_state( self_ptr() )
						{
							// ...
						}

					// ...

				};
			\endcode
		*/
		const agent_t *
		self_ptr() const;

		//! Начальное событие агента.
		/*! Т.е. то событие, которое гарантировано
			будет вызвано первым, в тот момент когда,
			агент будет привязан к диспетчеру.

			Метод играет роль своеобразного конструктора,
			в контексте SObjectizer, когда сначала конструируется
			сам объект агента, затем он привязывается к своему диспетчеру
			и начинает работу на нити диспетчера с вызова \с so_evt_start().

			\code
				class a_sample_t
					:
						public so_5::rt::agent_t
				{
					// ...
					virtual void
					so_evt_start();
					// ...
				};

				a_sample_t::so_evt_start()
				{
					std::cout << "first agent action on binded"
						" dispatcher" << std::endl;

					// Отправить инициирующие сообщение.
					m_mbox->deliver_message( ... );
				}
			\endcode
		*/
		virtual void
		so_evt_start();

		//! Завершающее событие агента.
		/*!
			Т.е. то событие, которое гарантировано
			будет вызвано последним, до того момента, как
			агент будет отвязан к диспетчеру.

			Метод играет роль своеобразного деструктора,
			в контексте SObjectizer, когда агент может выполнить
			какие-то действия на своей нити, еще до того, как
			будет вызван деструктор агента.

			\code
				class a_sample_t
					:
						public so_5::rt::agent_t
				{
					// ...
					virtual void
					so_evt_finish();
					// ...
				};

				a_sample_t::so_evt_finish()
				{
					std::cout << "last agent activity";

					if( so_current_state() == m_db_error_happened )
					{
						// Уничтожаем подключение к БД
						// в том же потоке в котором работали и
						// в котором произошла ошибка
						m_db.release();
					}
				}
			\endcode
		*/
		virtual void
		so_evt_finish();

		//! Получить ссылку на текущее состояние.
		inline const state_t &
		so_current_state() const
		{
			return *m_current_state_ptr;
		}

		//! Имя кооперации, к которой принадлежит агент.
		/*!
			\return Если агент принадлежит какой-либо кооперации,
			то вернет имя кооперации. Если же агент не является
			членом какой-либо кооперации, то выбросывается исключение.
		*/
		const std::string &
		so_coop_name() const;

		//! Добавить агенту слушателя,
		//! время жизни которого агент не контролирует.
		void
		so_add_nondestroyable_listener(
			agent_state_listener_t & state_listener );

		//! Добавить агенту слушателся,
		//! владение которым передается агенту.
		void
		so_add_destroyable_listener(
			agent_state_listener_unique_ptr_t state_listener );

		//! Поставить в очередь событие для выполнения агентом.
		/*!
			Метод который используется для планирования событий агента.
		*/
		static inline void
		call_push_event(
			agent_t & agent,
			//! Вызыватель обработичика.
			const event_caller_block_ref_t & event_handler_caller,
			//! Экземпляр сообщения, которое будет параметром
			//! обработчика событие, которое должен будет выполнить агент.
			const message_ref_t & message )
		{
			agent.push_event( event_handler_caller, message );
		}

		//! Инициировать выполнение очередного события.
		/*!
			Метод который используется диспетчерами (рабочими нитями),
			для выполнения событий агента.
		*/
		static inline void
		call_next_event(
			//! Агент, у которого в очереди есть события,
			//! и очередное событие которого надо выполнить.
			agent_t & agent )
		{
			agent.exec_next_event();
		}

		//! Привязать агента к диспетчеру.
		static inline void
		call_bind_to_disp(
			//! Агент.
			agent_t & agent,
			//! Диспетчер.
			dispatcher_t & disp )
		{
			agent.bind_to_disp( disp );
		}

	protected:
		//! Работа с состояниями.
		//! \{

		//! Получить ссылку на состояние по умолчанию.
		const state_t &
		so_default_state() const;

		//! Сменить состояние.
		/*!
			\code
				void
				a_sample_t::evt_smth(
					const so_5::rt::event_data_t< message_one_t > & msg )
				{
					// Если с сообщение что-то не так, то
					// переходим в сотояние "ошибка".
					if( error_in_data( *msg ) )
						so_change_state( m_error_state );
				}
			\endcode
		*/
		ret_code_t
		so_change_state(
			//! Новое состояние агента.
			const state_t & new_state,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );
		//! \}

		//! Подписка и отписка от сообщения.
		//! \{

		//! Создать объект для выполнения подписки.
		/*!
			\code
				void
				a_sample_t::so_define_agent()
				{
					// Подписаться на сообщения.
					so_subscribe( m_mbox_target )
						.in( m_state_one )
							.event( &a_sample_t::evt_sample_handler );
				}
			\endcode
		*/
		subscription_bind_t
		so_subscribe(
			//! Mbox на сообщение которого подписывать.
			const mbox_ref_t & mbox_ref );

		//! Создать объект для изьятия подписки.
		/*!
			\code
				void
				a_sample_t::evt_smth(
					const so_5::rt::event_data_t< message_one_t > & msg )
				{
					// Отписаться от сообщения.
					so_unsubscribe( m_mbox_target )
						.in( m_state_one )
							.event( &a_sample_t::evt_sample_handler );
				}
			\endcode
		*/
		subscription_unbind_t
		so_unsubscribe(
			//! mbox от сообщений которого отписывать.
			const mbox_ref_t & mbox_ref );
		//! \}

		//! Первоначальная инициализация агента.
		//! \{

		//! Метод определения агента.
		/*!
			Метод, который вызывается до того как
			агент начнет работать в SO, т.е. до того
			как начнется его работа на рабочей нити.
			\code
				class a_sample_t
					:
						public so_5::rt::agent_t
				{
					// ...
					virtual void
					so_define_agent();

					void
					evt_handler_1(
						const so_5::rt::event_data_t< message1_t > & msg );
					// ...

					void
					evt_handler_N(
						const so_5::rt::event_data_t< messageN_t > & msg );

				};

				void
				a_sample_t::so_define_agent()
				{
					// Подписываемся на сообщения.
					so_subscribe( m_mbox1 )
						.in( m_state_1 )
							.event( &a_sample_t::evt_handler_1 );
					// ...
					so_subscribe( m_mboxN )
						.in( m_state_N )
							.event( &a_sample_t::evt_handler_N );
				}
			\endcode
		*/
		virtual void
		so_define_agent();

		//! \}

		//! Был ли агент определен?
		/*!
			Когда надо определить определен ли агент уже.
			\code
				class a_sample_t
					:
						public so_5::rt::agent_t
				{
					// ...

					public:
						void
						set_target_mbox(
							const so_5::rt::mbox_ref_t & mbox )
						{
							// mbox для подписки нельзя менять
							// после того как агент на него подписался.

							if( !so_was_defined() && mbox.get() )
							{
								m_target_mbox = mbox;
							}
						}

					private:
						so_5::rt::mbox_ref_t m_target_mbox;
				};
			\endcode
		*/
		bool
		so_was_defined() const;

	public:
		//! Получить ссылку на среду SO к которой
		//! принадлежит агент.
		/*!
			Метод служит для получения доступа к
			SObjectizer Environment, а следовательно и ко всем методам
			SObjectizer Environment. Это, например позволяет агенту
			создать кооперации лругих агентов.
			\code
				void
				a_sample_t::evt_on_smth(
					const so_5::rt::event_data_t< some_message_t > & msg )
				{
					so_5::rt::agent_coop_unique_ptr_t coop =
						so_environment().create_coop(
							so_5::rt::nonempty_name_t( "first_coop" ) );

					// Добавляем в кооперацию агентов.
					coop->add_agent( so_5::rt::agent_ref_t(
						new a_another_t( ... ) ) );
					// ...

					// Регистрируем кооперацию.
					so_environment().register_coop( coop );
				}
			\endcode

			Или, например, инициировать завершение работы.
			\code
				void
				a_sample_t::evt_last_event(
					const so_5::rt::event_data_t< message_one_t > & msg )
				{
					// Обрабатываем сообщение.
					process( msg );

					// Останавливаемся.
					so_environment().stop();
				}
			\endcode
		*/
		so_environment_t &
		so_environment();

	private:
		//! Получить ссылку на агент.
		//! Внутренний метод SO.
		//! Вызывается только тогда - когда гарантированно агент еще
		//! нужен и его кто-то держит по ссылке.
		agent_ref_t
		create_ref();

		//! Внедрение агента в действующий SO Runtime.
		//! \{

		//! Привязать агента к кооперации.
		/*!
			Инициализирует внутренний указатель на кооперацию,
			а также взводит флаг что кооперация, которой принадлежит агент
			не является помеченной на дерегистрацию.
		*/
		void
		bind_to_coop(
			//! Кооперация агентов.
			agent_coop_t & coop );

		//! Привязать агента к среде so_5.
		/*!
			Инициализирует внутренний указатель на среду SO
			и получает свою локальную очередь.
			Вызывается из конструктора агента.
		*/
		void
		bind_to_environment(
			//! Среда so_5.
			impl::so_environment_impl_t & env_impl );

		//! Привязать агента к диспетчеру.
		/*!
			Определяет диспетчер, который будет являться
			реальным диспетчером агента.
			Если в локальной очереди что-то накопилось, то
			рельному диспетчеру будет поставлен запрос на выполнение
			всего количества накопившихся событий ко времени
			начала работы агента на данном диспетчере.
		*/
		void
		bind_to_disp(
			//! Диспетчер.
			dispatcher_t & disp );

		//! Внутренний метод определения агента.
		/*!
			Вызывает so_define_agent(), после чего
			устанавливает флаг, что агент определен.
		*/
		void
		define_agent();

		//! Метод изьятия агента из системы.
		/*!
			Метод который изымает все подписки агента.
			\note Внутренний метод SO.
		*/
		void
		undefine_agent();

		//! \}

		//! Добавление/удаление привязок для обработки сообщений.
		//! \{

		//! Создать привязку между агентом и mbox-ом
		//! для обработки сообщений.
		ret_code_t
		create_event_subscription(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Ссылка на mbox.
			mbox_ref_t & mbox_ref,
			//! Вызыватель обработчика.
			const event_handler_caller_ref_t & ehc,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );

		//! Уничтожить привязку между агентом и mbox-ом
		//! для обработки сообщений.
		ret_code_t
		destroy_event_subscription(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Mbox.
			mbox_ref_t & mbox_ref,
			//! Вызыватель обработчика.
			const event_handler_caller_ref_t & ehc,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );

		//! Уничтожить все подписки.
		void
		destroy_all_subscriptions();
		//! \}

		//! Постановка событий в очередь и их выполнение.
		//! \{

		//! Поставить в очередь локальное событие.
		void
		push_event(
			//! Вызыватель обработичика
			const event_caller_block_ref_t & event_handler_caller,
			//! Экземпляр сообщения.
			const message_ref_t & message );

		//! Выполнить очередное событие.
		/*!
			Должно вызываться только на контексте рабоченй нити.
			\note Полагается, что в очереди событий есть элементы.
		*/
		void
		exec_next_event();
		//! \}

		//! Состояние по умолчанию.
		const state_t m_default_state;

		//! Текущее состояние агента.
		const state_t * m_current_state_ptr;

		//! Был ли агент определен.
		bool m_was_defined;

		//! Контроллер слушателей агента.
		std::unique_ptr< impl::state_listener_controller_t >
			m_state_listener_controller;

		//! Тип карта привязок к обработчикам событий.
		typedef std::map<
				subscription_key_t,
				impl::message_consumer_link_t * >
			consumers_map_t;

		//! Потребители сообщения, которые являются
		//! обработчиками событий.
		consumers_map_t m_event_consumers_map;

		//! Локальная очередь событий.
		std::unique_ptr< impl::local_event_queue_t >
			m_local_event_queue;

		//! Среда so_5 которой принадлежит агент.
		impl::so_environment_impl_t * m_so_environment_impl;

		//! Диспетчер который занимается вызывом агента.
		/*! По умолчанию получает заглушку,
			которая ничего не делает, а просто поглащает
			вызовы постановки агента в очередь на вызов.
			После привязки агента к реальному диспетчеру
			сменить его нельзя, и при попытке это сделать
			произойдет ошибка.

			Владение над m_dispatcher агент не имеет.
		*/
		dispatcher_t * m_dispatcher;

		//! Кооперация которой принадлежит агент.
		agent_coop_t * m_agent_coop;

		//! Флаг помечена ли кооперация, как удаляемая?
		bool m_is_coop_deregistered;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
