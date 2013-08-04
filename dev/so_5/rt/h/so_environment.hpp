/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Среда исполнения so_5.
*/

#if !defined( _SO_5__RT__SO_ENVIRONMENT_HPP_ )
#define _SO_5__RT__SO_ENVIRONMENT_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/disp_binder.hpp>
#include <so_5/rt/h/so_layer.hpp>
#include <so_5/rt/h/coop_listener.hpp>
#include <so_5/rt/h/event_exception_logger.hpp>
#include <so_5/rt/h/event_exception_handler.hpp>

#include <so_5/timer_thread/h/timer_thread.hpp>
#include <so_5/timer_thread/h/timer_id.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class so_environment_impl_t;

} /* namespace impl */

class type_wrapper_t;
class message_ref_t;

//
// so_environment_params_t
//

//! Инициализирующие параметры среды so_5.
/*!
	С помощью данного класса задаются параметры SObjectizer Environment.
	Класс позволяет простым образом указать необходимые параметры
	оставляя другие по умолчанию.

	\see http://www.parashift.com/c++-faq/named-parameter-idiom.html
*/
class SO_5_TYPE so_environment_params_t
{
		friend class impl::so_environment_impl_t;

	public:
		so_environment_params_t();
		~so_environment_params_t();

		//! Указать размер пула мутексов для синхронизации работы mbox-ов.
		/*!
			Для синхронизации действий с mbox-ами необходимы
			ресурсы, для чего mbox использует мутекс (ACE_RW_Thread_Mutex ).
			Но т.к. mbox-ов может быть много, и время жизни их может быть
			достаточно коротким, то было бы не целесообразно в
			каждый mbox включать собственный метекс. Поэтому в
			SObjectizer Environment создается пул мутексов для использования
			обычными mbox-ами.

			\see so_environment_t::create_local_mbox().
		*/
		so_environment_params_t &
		mbox_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Указать размер пула мутексов для коопераций агентов.
		/*!
			Кооперации агентов требуют синхронизации.
			По рациональным соображениям создавать мутекс
			для каждой кооперации неэффективно. Поэтому
			SObjectizer Environment создает пул таких мутексов,
			которые и назначаются кооперациям.
		*/
		so_environment_params_t &
		agent_coop_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Указать размер пула мутексов для синхронизации
		//! локальных очередей событий агентов.
		/*!
			Локальная очередь событий агентов требует синхронизации.
			Свою локальную очередь агент получает во время создания от
			SObjectizer Environment, которая передается ему в конструкторе.
			Но, из-за того что агентов в системе может быть много,
			то выделять мутекс для каждой локальной очереди не рационально,
			поэтому SObjectizer Environment создает пул мутексов для
			обеспечения работы локальных очередей событий агентов.
		*/
		so_environment_params_t &
		agent_event_queue_mutex_pool_size(
			unsigned int mutex_pool_size );

		//! Добавить именованный диспетчер.
		/*!
			По умолчанию SObjectizer Environment имеет только один диспетчер
			с одной рабочей нитью. Пользователь может добавлять
			дополнительные -– именованные диспетчеры рабочих нитей.

			\note Если ранее уже добавлялся диспетчер с таким именем,
			то он будет замещен вновь добавленным.
		*/
		so_environment_params_t &
		add_named_dispatcher(
			//! Имя диспетчера.
			const nonempty_name_t & name,
			//! Диспетчер.
			dispatcher_unique_ptr_t && dispatcher );

		//! Добавить таймерную нить.
		/*!
			Если \a timer_thread указывает на 0, то
			будет использована таймерная нить по умолчанию.
		*/
		so_environment_params_t &
		timer_thread(
			//! Таймерная нить.
			so_5::timer_thread::timer_thread_unique_ptr_t && timer_thread );

		//! Добавление дополнительных слоев.
		/*!
			Если слой такого типа уже добавлен,
			то он будет замещен вновь добавляемым.
		*/
		template< class SO_LAYER >
		so_environment_params_t &
		add_layer(
			//! Слой SObjectizer, который должен быть включен
			//! в SObjectizer Environment.
			std::unique_ptr< SO_LAYER > && layer_ptr )
		{
			if( layer_ptr.get() )
			{
				so_layer_unique_ptr_t ptr( layer_ptr.release() );

				add_layer(
					type_wrapper_t( typeid( SO_LAYER ) ),
					std::move( ptr ) );
			}

			return *this;
		}

		so_environment_params_t &
		coop_listener(
			coop_listener_unique_ptr_t && coop_listener );

		so_environment_params_t &
		event_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		so_environment_params_t &
		event_exception_handler(
			event_exception_handler_unique_ptr_t && handler );

		unsigned int
		mbox_mutex_pool_size() const;

		unsigned int
		agent_coop_mutex_pool_size() const;

		unsigned int
		agent_event_queue_mutex_pool_size() const;

		const named_dispatcher_map_t &
		named_dispatcher_map() const;

		const so_layer_map_t &
		so_layers_map() const;

	private:
		//! Добавить новый слой.
		/*!
			Если слой такого типа уже добавлен,
			то он будет замещен вновь добавляемым.
		*/
		void
		add_layer(
			//! Информация о типе слоя.
			const type_wrapper_t & type,
			//! Слой.
			so_layer_unique_ptr_t layer_ptr );

		//! Размер пула мутексов для mbox-ов.
		unsigned int m_mbox_mutex_pool_size;

		//! Размер пула мутексов для коопераций агентов агентов.
		unsigned int m_agent_coop_mutex_pool_size;

		//! Размер пула мутексов для локальных очередей событий агентов.
		unsigned int m_agent_event_queue_mutex_pool_size;

		//! Именованные диспетчеры.
		named_dispatcher_map_t m_named_dispatcher_map;

		//! Таймерная нить.
		so_5::timer_thread::timer_thread_unique_ptr_t m_timer_thread;

		//! Дополнительные слои SObjectizer.
		so_layer_map_t m_so_layers;

		//! Слущатель действий над кооперациями.
		coop_listener_unique_ptr_t m_coop_listener;

		//! Логер исключений.
		event_exception_logger_unique_ptr_t m_event_exception_logger;

		//! Обработчик исключений.
		event_exception_handler_unique_ptr_t m_event_exception_handler;
};

//
// so_environment_t
//

//! Среда выполнения SObjectizer.
/*!
	Среда выполнения SObjectizer предоставляет необходимую для агентов
	инфраструктуру. Она является фундаментом,
	на котором работают SObjectizer агенты.

	Для создания и запуска SObjectizer Environment предполагается,
	что прикладной программист создает собственный класс наследуясь
	от so_environment_t и переопределяет метод
	so_environment_t::init().
	Внутри этого метода программист определяет логику начала работы
	SObjectizer Environment, например, регистрация первоначальных коопераций
	и отправку инициализируеющих сообщений.
	В момент вызова init() SObjectizer Environment уже успешно инициализировал
	и запустил необходимую инфраструктуру, иначе до вызова init() дело не дойдет.

	Для непосредственного запуска SObjectizer Environment служит метод
	so_environment_t::run(), который блокирует вызывающую нить
	на протяжении всего времени работы SObjectizer.
	Если ошибок не произошло, то метод so_environment_t::run() возвращает 0.
	Если во время работы SObjectizer Environment произошла ошибка,
	то вернет ее код или выбросит исключение. Это зависит от параметра
	\a throwing_strategy (so_5::throwing_strategy_t).

	Для остановки работы SObjectizer Environment служит метод
	so_environment_t::stop().
	Вызов so_environment_t::stop() только инициирует завершение работы
	SObjectizer Environment. Этот метод инициирует дерегистрацию всех
	коопераций. А когда все кооперации будут дерегистрированы,
	будет завершена работы внутренних компонент SObjectizer Environment.
	После этого работа SObjectizer будет завершена.

	Во время работы SObjectizer Environment предоставляет
	следующие три группы прикладных методов:
		\li методы работы с mbox-ами;
		\li методы работы с кооперациями;
		\li методы отправки отложенных и периодических сообщений.

	<b>Методы работы с mbox-ами.</b>

	SObjectizer Environment позволяет создавать
	безымянные mbox-ы с ресурсами синхронизации из пула или
	указанных пользователем, или также именные mbox, список которых
	ведет SObjectizer Environment.

	Для создания mbox-ов служат методы so_environment_t::create_local_mbox().
	Все они возвращают объект класса mbox_ref_t, который является указателем
	с подсчетом ссылок на объект класса mbox_t.
	Уничтожение безымянных mbox-ов происходит само собой, когда ссылок на него
	не остается.
	Именованные mbox-ы же хранятся в SObjectizer Environment, что дает
	постоянную ссылку на mbox. Для того чтобы изъять именованный mbox из
	этого списка, служит метод so_environment_t::destroy_mbox().
	Этот метод исключает именованный mbox из списка и, если на него больше никто не
	ссылается, то он будет уничтожен.

	\note Если после исключение именнованный mbox остается, например,
	у какого-либо агента, а затем создается именованный mbox
	с таким же именем то это это будут разные mbox-ы. И сообщения,
	посылаемые через один mbox, не будут передаваться подписчикам другого.

	<b>Методы работы с кооперациями.</b>

	Кооперации можно создавать с помощью методов so_environment_t::create_coop(),
	для регистрации кооперации служит метод so_environment_t::register_coop(),
	а для дерегистрации кооперации служит метод
	so_environment_t::deregister_coop().

	<b>Методы отправки отложенных и периодических сообщений</b>

	Отправка отложенного и периодического сообщения называется таймерным событием.
	Механизм таймерных событий позволяет не только заводить,
	но и отменять таймерные события. В случае отложенного сообщения оно
	не будет отправлено, если, конечно, оно еще не было отправлено, а в случае
	периодического сообщения – отправка его будет прекращена.
	Для создания таймерных событий служит метод
	so_environment_t::schedule_timer(), который имеет две версии,
	первая подразумевает, что передается реальный экземпляр сообщения,
	а второй предусматривает только тип сообщения без реального
	экземпляра сообщения.
	so_environment_t::schedule_timer() возвращают таймерный
	идентификатор — объект типа so_5::timer_thread::timer_id_ref_t,
	который хранит данные, необходимые для отмены таймерного события.
	А отменить таймерное событие можно либо с помощью метода
	so_5::timer_thread::timer_id_ref_t::release(), либо
	оно будет уничтожено автоматически, если все ссылки на таймерное
	событие уничтожены.

	В некоторых случаях есть необходимость отправить отложенное сообщение,
	которое ни в каких случаях не требует отмены, но
	so_environment_t::schedule_timer() требует запоминать таймерный
	идентификатор, в противном случае, отправка отложенного сообщения будет
	тут же отменена. Чтобы избежать хранения таймерного
	идентификатора в таких случаях в SObjectizer Environment предусмотрен
	метод so_environment_t::single_timer(), который планирует отложенную
	отправку сообщения, для которой не надо хранить таймерный идентификатор.
*/
class SO_5_TYPE so_environment_t
{
		//! Получить ссылку на себя.
		/*!
			Служит для инициализации внутренних переменных
			в конструкторе so_environment_t, конструктор которых
			требует указать ссылку на so_environment_t.
			Это позволяет избежать предупреждений компилятора
			об использовании \c this.
		*/
		so_environment_t &
		self_ref();

	public:
		explicit so_environment_t(
			//! Параметры среды SObjectizer.
			const so_environment_params_t & so_environment_params );

		virtual ~so_environment_t();

		//! \name Работа с mbox-ами.
		//! \{

		//! Создать безымянный mbox.
		/*!
			Мутекс для синхронизации берется из общего пула.
		*/
		mbox_ref_t
		create_local_mbox();

		//! Создать именованный mbox.
		/*!
			Если mbox-а с таким именем нет, то он создается.
			Если же такой mbox уже был создан, то просто
			возвращается сылка на него.
			Мутекс для синхронизации берется из общего пула.
		*/
		mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & mbox_name );

		//! Создать безымянный mbox,
		//! мутекс для синхронизации создается пользователем.
		/*!
			Есть и особые случаи, когда надо обеспечить,
			чтобы mbox синхронизировался без параллельного использования
			в тех же целях другими mbox-ами. В таких случаях, создавая mbox,
			можно указать собственный динамически созданный мутекс
			(ACE_RW_Thread_Mutex).
		*/
		mbox_ref_t
		create_local_mbox(
			//! Мутекс созданный пользователем.
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );

		//! Создать именованный mbox.
		/*!
			Если mbox-а с таким именем нет, то он создается.
			Если же такой mbox уже был создан, то просто
			возвращается сылка на него.
			Мутекс для синхронизации создается пользователем.
		*/
		mbox_ref_t
		create_local_mbox(
			//! Имя mbox-а.
			const nonempty_name_t & mbox_name,
			//! Мутекс созданный пользователем
			std::unique_ptr< ACE_RW_Thread_Mutex > lock_ptr );
		//! \}

		//! \name Диспетчеры.
		//! \{

		//! Получить диспетчер по умолчанию.
		dispatcher_t &
		query_default_dispatcher();

		//! Получить именованный диспетчер.
		/*!
			\return Если диспетчер с таким именем есть,
			то вернется dispatcher_ref_t, который указывает на него,
			в противном случае вернется dispatcher_ref_t на 0.
		*/
		dispatcher_ref_t
		query_named_dispatcher(
			//! Имя диспетчера.
			const std::string & disp_name );

		//! Установить логер исключений.
		void
		install_exception_logger(
			event_exception_logger_unique_ptr_t && logger );

		//! Установить обработчик исключений.
		void
		install_exception_handler(
			event_exception_handler_unique_ptr_t && handler );
		//! \}

		//! \name Работа с кооперациями.
		//! \{

		//! Создать кооперацию.
		/*!
			Создается кооперация с заданным именем,
			с классом привязки агентов к диспетчеру SObjectizer по умолчанию.
		*/
		agent_coop_unique_ptr_t
		create_coop(
			//! Имя кооперации.
			const nonempty_name_t & name );

		//! Создать кооперацию.
		/*!
			По умолчанию агенты кооперации привязываются
			к диспетчеру с помощью \a disp_binder.

			Используется для тех агентов, которые добавляютяс к
			кооперации без явного указания привязки к диспетчеру.
			\code
				so_5::rt::agent_coop_unique_ptr_t coop = so_env.create_coop(
					so_5::rt::nonempty_name_t( "some_coop" ),
					so_5::disp::active_group::create_disp_binder(
						"active_group",
						"some_active_group" ) );

				// Агент будет привязан к деспетчеру активных групп "active_group"
				// и будет работать на нити активной группы "some_active_group"
				coop->add_agent(
					so_5::rt::agent_ref_t( new a_some_agent_t( env ) ) );
			\endcode
			*/
		agent_coop_unique_ptr_t
		create_coop(
			//! Имя кооперации.
			const nonempty_name_t & name,
			//! Привязка  агентов к диспетчеру, используемая
			//! по умолчанию для агентов кооперации.
			disp_binder_unique_ptr_t disp_binder );

		//! Зарегистрировать кооперацию.
		/*!
			При регистрации агенты сначала привязываются к кооперации,
			после этого агенты кооперации знают в какую кооперацию они входят.
			Затем SObjectizer Environment проверяет: является ли имя
			кооперации уникальным и, если имя не уникальное,
			то это приводит к ошибке. Если имя уникальное, то дальше
			осуществляется определение агентов, у них вызывается метод
			agent_t::so_define_agent(), что позволяет агенту подписаться на
			сообщения, еще до того как агент привязан к определенной рабочей нити.
			И в последнюю очередь происходит привязка агентов к их диспетчерам.
			Если все агенты успешно привязались к своим диспетчерам,
			то кооперация становится успешно зарегистрированной
			и добавляется в словарь зарегистрированных коопераций.
			В случае успешной регистрации register_coop вернет 0.
		*/
		ret_code_t
		register_coop(
			//! Кооперация, агентов которой надо зарегистрировать.
			agent_coop_unique_ptr_t agent_coop,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Дерегистрировать кооперацию.
		/*!
			При дерегистрации SObjectizer Environment проверяет наличие
			кооперации в словаре зарегистрированных коопераций и,
			если она находится, то инициирует ее дерегистрацию.

			Важно понимать, что дерегистрация кооперации не значит,
			что ее агенты немедленно перестают работать,
			в этот момент агенты уведомляются, что они дерегистрируются.
			Получив такое уведомление, агенты перестают принимать новые заявки
			на обработку сообщений. И только когда агенты обработают
			локальную очередь событий до конца, они уведомят об этом кооперацию.
			Кооперация, в свою очередь, приполучении сигнала от всех своих
			агентов, уведомляет об этом SObjectizer,
			который затем на отдельной служебной нити окончательно
			дерегистрирует кооперацию.

			После окончательной дерегистрации кооперации, агенты отвязываются
			от диспетчеров. После этого кооперация удаляется из
			словаря зарегистрированных коопераций.
		*/
		ret_code_t
		deregister_coop(
			//! Имя дерегистрируемой кооперации.
			const nonempty_name_t & name,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy =
				DO_NOT_THROW_ON_ERROR );

		//! \}

		//! \name Работа с таймерными событиями.
		//! \{

		//! Запланировать таймерное событие.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Сообщение.
			std::unique_ptr< MESSAGE > & msg,
			//! Mbox, на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед первой отправкой сообщения.
			unsigned int delay_msec,
			//! Период отправки сообщений. Для отложенных сообщений
			//! \a period_msec равно 0.
			unsigned int period_msec )
		{
			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Запланировать таймерное событие.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Сообщение.
			std::unique_ptr< MESSAGE > && msg,
			//! Mbox, на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед первой отправкой сообщения.
			unsigned int delay_msec,
			//! Период отправки сообщений. Для отложенных сообщений
			//! \a period_msec равно 0.
			unsigned int period_msec )
		{
			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Запланировать таймерное событие с пустым сообщением.
		template< class MESSAGE >
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Mbox, на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед первой отправкой сообщения.
			unsigned int delay_msec,
			//! Период отправки сообщений. Для отложенных сообщений
			//! \a period_msec равно 0.
			unsigned int period_msec )
		{
			return schedule_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t(),
				mbox,
				delay_msec,
				period_msec );
		}

		//! Запланировать единичное таймерное событие,
		//! которое нельзя отменить.
		template< class MESSAGE >
		void
		single_timer(
			//! Сообщение.
			std::unique_ptr< MESSAGE > msg,
			//! Mbox, на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед отправкой сообщения.
			unsigned int delay_msec )
		{
			single_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t( msg.release() ),
				mbox,
				delay_msec );
		}

		//! Запланировать единичное таймерное событие,
		//! которое нельзя отменить, с пустым сообщением.
		template< class MESSAGE >
		void
		single_timer(
			//! Mbox, на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед отправкой сообщения.
			unsigned int delay_msec )
		{
			single_timer(
				type_wrapper_t( typeid( MESSAGE ) ),
				message_ref_t(),
				mbox,
				delay_msec );
		}
		//! \}

		//! \name Работа со слоями.
		//! \{

		//! Получить указатель на требумый слой.
		template< class SO_LAYER >
		SO_LAYER *
		query_layer(
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR ) const
		{
			// Проверка на полиморфность SO_LAYER типу so_layer_t
			// которая происходит во время компиляции.
			so_layer_t * layer = static_cast< so_layer_t* >( (SO_LAYER *)0 );

			layer = query_layer(
				type_wrapper_t( typeid( SO_LAYER ) ) );

			if( layer )
				return dynamic_cast< SO_LAYER * >( layer );

			if( THROW_ON_ERROR == throwing_strategy )
				throw exception_t(
					"layer does not exist",
					rc_layer_does_not_exist );

			return nullptr;
		}

		//! Добавить дополнительный слой.
		template< class SO_LAYER >
		ret_code_t
		add_extra_layer(
			std::unique_ptr< SO_LAYER > layer_ptr,
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			return add_extra_layer(
				type_wrapper_t( typeid( SO_LAYER ) ),
				so_layer_ref_t( layer_ptr.release() ),
				throwing_strategy );
		}
		//! \}

		//! \name Запуск, инициализация и остановка.
		//! \{
		//! Запустить среду SO_5.
		ret_code_t
		run(
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy =
				THROW_ON_ERROR );

		//! Инициализировать среду SO.
		//! Метод вызывается тогда, когда среда уже запущена.
		/*!
			\attention Для того чтобы комманды
			на остановку среды Sobjectizer (метод stop())
			возымели действие необходимо
			чтобы работа init() завершилась. В противном случае
			управляющая нить среды Sobjectizer, остается внутри
			init(), и не может приступить к ожиданию
			завершения работы, а соответственно дождаться его и
			начать завершение работы Sobjectizer. Например, если
			в init() сначала регистрируется кооперация,
			ее агенты начинают работать на своих диспетчерах рабочих нитей,
			а init() далее начинает бесконечный диалог с пользователем.
			Тогда даже если како-либо агент вызовет so_environment_t::stop(), то
			среда Sobjectizer продолжит работать в том же режиме,
			что и до этого, оставаясь полностью рабочей.
		*/
		virtual void
		init() = 0;

		//! Завершить выполнение среды SO_5.
		void
		stop();
		//! \}

		//! Получить реализацию so_environment_impl.
		impl::so_environment_impl_t &
		so_environment_impl();

	private:
		//! Запланировать таймерное событие.
		so_5::timer_thread::timer_id_ref_t
		schedule_timer(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Сообщение.
			const message_ref_t & msg,
			//! mbox на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед первой отправкой сообщения.
			unsigned int delay_msec,
			//! Период отправки сообщений, для отложенных сообщений
			//! \a period_msec == 0.
			unsigned int period_msec = 0 );

		//! Запланировать единичное таймерное событие,
		//! которое нельзя отменить.
		void
		single_timer(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! Сообщение.
			const message_ref_t & msg,
			//! mbox на который надо отсылать сообщение.
			const mbox_ref_t & mbox,
			//! Задержка перед отправкой сообщений.
			unsigned int delay_msec );

		//! Получить слой.
		so_layer_t *
		query_layer(
			const type_wrapper_t & type ) const;

		//! Добавить дополнительный слой.
		ret_code_t
		add_extra_layer(
			const type_wrapper_t & type,
			const so_layer_ref_t & layer,
			throwing_strategy_t throwing_strategy );

		//! Изьять дополнительный слой.
		ret_code_t
		remove_extra_layer(
			//! Тип слоя.
			const type_wrapper_t & type,
			throwing_strategy_t throwing_strategy );

		//! Реализация среды so_5.
		impl::so_environment_impl_t * m_so_environment_impl;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
