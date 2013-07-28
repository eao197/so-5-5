/*
	SObjectizer 5.
*/

#include <ace/Guard_T.h>

#include <so_5/util/h/apply_throwing_strategy.hpp>

#include <so_5/rt/h/agent.hpp>
#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/so_environment.hpp>
#include <so_5/rt/h/event_caller_block.hpp>

#include <so_5/rt/impl/h/cmp_method_ptr.hpp>
#include <so_5/rt/impl/h/message_consumer_link.hpp>
#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/impl/h/local_event_queue.hpp>
#include <so_5/rt/impl/h/void_dispatcher.hpp>
#include <so_5/rt/impl/h/state_listener_controller.hpp>

namespace so_5
{

namespace rt
{

namespace /* ananymous */
{

//
// hook_event_caller_t
//

//! Базовый класс для вызова начального и завершающего события.
class hook_event_caller_t
	:
		public event_handler_caller_t
{
	public:

		hook_event_caller_t(
			agent_t & agent )
			:
				m_agent( agent )
		{}

		virtual ~hook_event_caller_t()
		{}

		//! Реализация методов базового класса.
		//! \{
		virtual char *
		ordinal() const
		{
			// Этот метод никогда не должен вызываться у данного класса.
			std::abort();
			return 0;
		}

		virtual size_t
		ordinal_size() const
		{
			// Этот метод никогда не должен вызываться у данного класса.
			std::abort();
			return 0;
		}

		virtual type_wrapper_t
		type_wrapper() const
		{
			// Этот метод никогда не должен вызываться у данного класса.
			std::abort();
			type_wrapper_t fake( typeid( message_t ) );
			return fake;
		};
		//! \}

	protected:
		agent_t & m_agent;
};

//
// start_hook_event_caller_t
//

//! Событие, которое добавляется в очередь первым
//! чтобы вызывать у агента evt_start().
class start_hook_event_caller_t
	:
		public hook_event_caller_t
{
	public:

		start_hook_event_caller_t(
			agent_t & agent )
			:
				hook_event_caller_t( agent )
		{}
		virtual ~start_hook_event_caller_t()
		{}

		//! Реализация методов базового класса.
		//! \{

		//! Вызвать у агента метод обработки события.
		/*!
			Вызывает начальное событие агента so_evt_start().
		*/
		virtual bool
		call(
			message_ref_t & ) const
		{
			// Вызываем начальное событие.
			m_agent.so_evt_start();
			return true;
		}
		//! \}
};

//
// finish_hook_event_caller_t
//

//! Событие терминатор, которое вызывает so_evt_finish().
class finish_hook_event_caller_t
	:
		public hook_event_caller_t
{
	public:

		finish_hook_event_caller_t(
			agent_t & agent )
			:
				hook_event_caller_t( agent )
		{}
		virtual ~finish_hook_event_caller_t()
		{}

		//! Реализация методов базового класса.
		//! \{

		//! Вызвать у агента метод обработки события.
		/*!
			Вызывает начальное событие агента so_evt_start().
		*/
		virtual bool
		call(
			message_ref_t & ) const
		{
			// Вызываем начальное событие.
			m_agent.so_evt_finish();
			return true;
		}
		//! \}
};

// Пустой диспетчер.
impl::void_dispatcher_t g_void_dispatcher;

}/* ananymous namespace */

//
// agent_t
//

agent_t::agent_t(
	so_environment_t & env )
	:
		m_default_state( self_ptr() ),
		m_current_state_ptr( &m_default_state ),
		m_was_defined( false ),
		m_state_listener_controller(
			new impl::state_listener_controller_t ),
		m_so_environment_impl( 0 ),
		m_dispatcher( &g_void_dispatcher ),
		m_agent_coop( 0 ),
		m_is_coop_deregistered( false )
{
	// Привязываемся к реализации среды SO.
	bind_to_environment( env.so_environment_impl() );
}

agent_t::~agent_t()
{
}

const agent_t *
agent_t::self_ptr() const
{
	return this;
}

void
agent_t::so_evt_start()
{
	// Реализация по умолчанию - ничего не делает.
}

void
agent_t::so_evt_finish()
{
	// Реализация по умолчанию - ничего не делает.
}


const std::string &
agent_t::so_coop_name() const
{
	if( 0 == m_agent_coop )
		throw exception_t(
			"agent isn't bind to cooperation yet",
			rc_agent_has_no_cooperation );

	return m_agent_coop->query_coop_name();
}

void
agent_t::so_add_nondestroyable_listener(
	agent_state_listener_t & state_listener )
{
	m_state_listener_controller->so_add_nondestroyable_listener(
		state_listener );
}

void
agent_t::so_add_destroyable_listener(
	agent_state_listener_unique_ptr_t state_listener )
{
	m_state_listener_controller->so_add_destroyable_listener(
		std::move( state_listener ) );
}

const state_t &
agent_t::so_default_state() const
{
	return m_default_state;
}

ret_code_t
agent_t::so_change_state(
	const state_t & new_state,
	throwing_strategy_t throwing_strategy )
{
	if( new_state.is_target( this ) )
	{
		// Меняем текущее состояние.
		m_current_state_ptr = &new_state;

		// Вызываем обработчики слушателей состояния.
		m_state_listener_controller->changed(
			*this,
			*m_current_state_ptr );

		return 0;
	}

	return so_5::util::apply_throwing_strategy(
		rc_agent_unknown_state,
		throwing_strategy,
		"unable to switch agent to alien state "
		"(the state that doesn't belong to this agent)" );
}

subscription_bind_t
agent_t::so_subscribe( const mbox_ref_t & mbox_ref )
{
	return subscription_bind_t( *this, mbox_ref );
}

subscription_unbind_t
agent_t::so_unsubscribe( const mbox_ref_t & mbox_ref )
{
	return subscription_unbind_t( *this, mbox_ref );
}

void
agent_t::so_define_agent()
{
	// Реализация по умолчанию - ничего не делает.
}

bool
agent_t::so_was_defined() const
{
	return m_was_defined;
}

void
agent_t::define_agent()
{
	so_define_agent();
	m_was_defined = true;
}

void
agent_t::undefine_agent()
{
	{
		// Лочим локальную очередь событий.
		ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

		m_is_coop_deregistered = true;

		if( m_dispatcher )
		{
			// Добавляем финальное событие в очередь.
			event_handler_caller_ref_t evt_caller(
				new finish_hook_event_caller_t( *this ) );

			event_caller_block_ref_t event_caller_block(
				new event_caller_block_t );

			event_caller_block->insert( evt_caller );

			m_local_event_queue->push(
				impl::event_item_t(
					event_caller_block,
					message_ref_t() ) );

			// Говорим своему диспетчеру, что у агента
			// появилось событие.
			m_dispatcher->put_event_execution_request(
				create_ref(), 1 );
		}
		else
		{
			// Если реальный диспетчер привязан не был,
			// то нет смысла ждать пока выполняться события,
			// так что мы их просто удаляем.
			m_local_event_queue->clear();
		}
	}

	// Избавляемся от подписок.
	destroy_all_subscriptions();
}

so_environment_t &
agent_t::so_environment()
{
	return m_so_environment_impl->query_public_so_environment();
}

agent_ref_t
agent_t::create_ref()
{
	agent_ref_t agent_ref( this );
	return agent_ref;
}

void
agent_t::bind_to_coop(
	agent_coop_t & coop )
{
	// Запоминаем кооперацию.
	m_agent_coop = &coop;
	// Выставляем флаг, что мы не дерегистрируемся.
	m_is_coop_deregistered = false;
}

inline void
agent_t::bind_to_environment(
	impl::so_environment_impl_t & env_impl )
{
		m_so_environment_impl = &env_impl;
		// Получаем очередь.
		m_local_event_queue = m_so_environment_impl->create_local_queue();

		// Добавляем начальное событие в очередь.
		event_handler_caller_ref_t evt_caller(
			new start_hook_event_caller_t( *this ) );

		event_caller_block_ref_t event_caller_block(
			new event_caller_block_t );

		event_caller_block->insert( evt_caller );

		m_local_event_queue->push(
			impl::event_item_t(
				event_caller_block,
				message_ref_t() ) );
}

void
agent_t::bind_to_disp(
	dispatcher_t & disp )
{
	// Лочим локальную очередь событий.
	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	// Проверяем стоит ли сейчас диспетчер-заглуша.
	// Если стоит не диспетчер заглушка, то это ошибка.
	if( m_dispatcher != &g_void_dispatcher )
	{
		throw exception_t(
			"agent is already bind to dispatcher",
			rc_agent_is_already_bind_to_disp );
	}

	// Раз стоит диспетчер заглушка, то будем менять его.
	m_dispatcher = &disp;
	// т.к. в локальной очереди уже могло накопиться какое-то
	// количество событий, то надо указать новому диспетчеру
	// о необходимости их выполнить.
	m_dispatcher->put_event_execution_request(
		create_ref(),
		m_local_event_queue->size() );
}

// Сделеть строковое представление ключа подписки.
inline std::string
subscription_key_string( const subscription_key_t & sk )
{
	const std::string msg_type =
		sk.m_type_wrapper.query_type_info().name();

	std::string str = "message type: " +
		msg_type + "; mbox: " + sk.m_mbox->query_name();

	return str;
}

ret_code_t
agent_t::create_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc,
	throwing_strategy_t throwing_strategy )
{
	subscription_key_t subscr_key(
		type_wrapper,
		mbox_ref );

	ACE_Guard< ACE_Thread_Mutex > lock( m_agent_coop->m_lock );
	// Если было инициировано разопределение агентов,
	// то подписку выполнять не нужно.
	if( m_agent_coop->m_agents_are_undefined )
		return 0;

	// Ищем такую подписку.
	consumers_map_t::iterator it =
		m_event_consumers_map.find( subscr_key );

	// Если такой нету, то пробуем ее добавить.
	if( m_event_consumers_map.end() == it )
	{
		// Создаем привязку.
		impl::message_consumer_link_t * message_consumer_link_ptr =
			new impl::message_consumer_link_t( create_ref() );

		std::unique_ptr< impl::message_consumer_link_t >
			message_consumer_link( message_consumer_link_ptr );

		mbox_ref->subscribe_first_event_handler(
			type_wrapper,
			message_consumer_link,
			ehc );

		m_event_consumers_map.insert(
			consumers_map_t::value_type(
				subscr_key,
				message_consumer_link_ptr ) );
	}
	else
	{
		return mbox_ref->subscribe_more_event_handler(
			type_wrapper,
			it->second,
			ehc,
			throwing_strategy );
	}

	return 0;
}

ret_code_t
agent_t::destroy_event_subscription(
	const type_wrapper_t & type_wrapper,
	mbox_ref_t & mbox_ref,
	const event_handler_caller_ref_t & ehc,
	throwing_strategy_t throwing_strategy )
{
	subscription_key_t subscr_key(
		type_wrapper,
		mbox_ref );

	ACE_Guard< ACE_Thread_Mutex > lock( m_agent_coop->m_lock );

	// Ищем такую подписку.
	consumers_map_t::iterator it =
		m_event_consumers_map.find( subscr_key );

	// Если такой подписки нет, то это ошибка.
	if( m_event_consumers_map.end() == it )
	{
		const std::string msg_type =
			type_wrapper.query_type_info().name();

		return so_5::util::apply_throwing_strategy(
			rc_no_event_handler_provided,
			throwing_strategy,
			"no event handler provided for this params "
			"(message type, mbox), where " +
			subscription_key_string( subscr_key ) );
	}

	// Будет ли удаляемая подписка последней?
	bool is_last_subscription = false;
	ret_code_t rc = mbox_ref->unsubscribe_event_handler(
		type_wrapper,
		it->second,
		ehc,
		is_last_subscription,
		throwing_strategy );

	if( 0 != rc )
		return rc;

	// Если для данного типа сообщения у данного
	// mbox - это последняя подписка для этого агента, то
	// надо удалить ее из списка.
	if( is_last_subscription )
	{
		m_event_consumers_map.erase( it );
	}

	return 0;
}

void
agent_t::destroy_all_subscriptions()
{
	ACE_Guard< ACE_Thread_Mutex > lock( m_agent_coop->m_lock );

	consumers_map_t::iterator it = m_event_consumers_map.begin();
	consumers_map_t::iterator it_end = m_event_consumers_map.end();

	for(; it != it_end; ++it )
	{
		mbox_ref_t mbox( it->first.m_mbox );
		// Исключаем подписку.
		mbox->unsubscribe_event_handler(
			it->first.m_type_wrapper,
			it->second );
	}
	m_event_consumers_map.clear();
}

void
agent_t::push_event(
	const event_caller_block_ref_t & event_caller_block,
	const message_ref_t & message )
{
	// Лочим локальную очередь событий.
	ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

	// Если агент не дерегистрирован, то добавляем ему сообщение.
	if( !m_is_coop_deregistered )
	{
		// сначала кладем событие в очередь.
		m_local_event_queue->push(
			impl::event_item_t( event_caller_block, message ) );

		// Если агент уже привязан к диспетчеру,
		// то сообщаем диспетчеру, что у агента появилось событие.
		if( m_dispatcher )
			m_dispatcher->put_event_execution_request(
				create_ref(), 1 );

		// Если агент не еще не привязан к диспетчеру,
		// то обработка сообщения будет
		// заявлена диспетчеру в bind_to_disp(),
		// в котором ставится столько заявок,
		// сколько накопилось событий в очереди.
	}
}

void
agent_t::exec_next_event()
{
	impl::event_item_t event_item;
	bool agent_finished_his_work_totally = false;
	{
		// Лочим локальную очередь событий.
		ACE_Guard< ACE_Thread_Mutex > lock( m_local_event_queue->lock() );

		// Берем очередное событие.
		m_local_event_queue->pop( event_item );

		// Устанавливаем флаг, надо ли информировать кооперацию,
		// о том что мы выполнили последнее событие.
		agent_finished_his_work_totally =
			m_is_coop_deregistered && 0 == m_local_event_queue->size();
	}

	// Выполняем событие.
	event_item.m_event_caller_block->call(
		event_item.m_message_ref );

	// Если агент полностью закончил работу,
	// то удаляем все подписки и уведомляем кооперацию.
	if( agent_finished_his_work_totally )
	{
		agent_coop_t::call_agent_finished( *m_agent_coop );
	}
}

} /* namespace rt */

} /* namespace so_5 */
