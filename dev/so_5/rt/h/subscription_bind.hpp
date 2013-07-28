/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Шаблонный класс для подписки агентов на сообщения.
*/

#if !defined( _SO_5__RT__SUBSCRIPTION_BIND_HPP_ )
#define _SO_5__RT__SUBSCRIPTION_BIND_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>
#include <so_5/h/types.hpp>
#include <so_5/h/throwing_strategy.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/state.hpp>
#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/event_data.hpp>
#include <so_5/rt/h/event_handler_caller.hpp>
#include <so_5/rt/h/event_handler_caller_ref.hpp>

namespace so_5
{

namespace rt
{

//
// agent_owns_state()
//

//! Проверить является ли агент владельцем состояния.
SO_5_EXPORT_FUNC_SPEC( ret_code_t )
agent_owns_state(
	agent_t & agent,
	const state_t * state,
	//! Флаг - бросать ли исключение в случае ошибки.
	throwing_strategy_t throwing_strategy );

//! Проверить является ли агент приводимым к данному типу.
template< class AGENT >
ret_code_t
agent_convertable_to(
	//! Агент, приводимость которого к типу AGENT проверяется.
	agent_t * agent,
	AGENT * & casted_agent,
	//! Флаг - бросать ли исключение в случае ошибки.
	throwing_strategy_t throwing_strategy )
{
	ret_code_t res = 0;
	casted_agent = dynamic_cast< AGENT * >( agent );

	// Если агент не приводится к заданному типу,
	// то это будет ошибкой.
	if( nullptr == casted_agent )
	{
		// Агент не является заданным типом.
		res = rc_agent_incompatible_type_conversion;

		// Если надо бросать исключение, то бросаем его.
		if( THROW_ON_ERROR == throwing_strategy )
		{
			std::string error_msg = "Unable convert agent to type ";
			const std::type_info & ti = typeid( AGENT );
			error_msg += ti.name();
			throw exception_t( error_msg, res );
		}
	}

	return res;
}

//
// subscription_bind_t
//

/*!
	Класс для создании подписки агента на сообщения от mbox.
*/
class SO_5_TYPE subscription_bind_t
{
	public:
		subscription_bind_t(
			agent_t & agent,
			const mbox_ref_t & mbox_ref );

		~subscription_bind_t();

		//! Определить состояние в котором
		//! должно произойти событие.
		subscription_bind_t &
		in(
			//! Состояние агента в котором возможна обработка сообщения.
			const state_t & state );

		//! Подписаться на сообщение.
		template< class MESSAGE, class AGENT >
		ret_code_t
		event(
			//! Метод обработки сообщения.
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			// Проверяем чтобы агент являлся владельцем состояния.
			ret_code_t res = agent_owns_state(
				m_agent,
				m_state,
				throwing_strategy );

			if( res )
				return res;

			AGENT * casted_agent = nullptr;
			// Проверяем чтобы агент был агентом заданного типа.
			res = agent_convertable_to< AGENT >(
				&m_agent,
				casted_agent,
				throwing_strategy );

			if( res )
				return res;

			event_handler_caller_ref_t event_handler_caller_ref(
				new real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					*casted_agent,
					m_state ) );

			return create_event_subscription(
				quick_typeid_t< MESSAGE >::m_type_wrapper,
				m_mbox_ref,
				event_handler_caller_ref,
				throwing_strategy );
		}

		//! Подписаться на сообщение.
		template< class MESSAGE, class AGENT >
		ret_code_t
		event(
			//! Метод обработки сообщения.
			void (AGENT::*pfn)( const not_null_event_data_t< MESSAGE > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			// Проверяем чтобы агент являлся владельцем состояния.
			ret_code_t res = agent_owns_state(
				m_agent,
				m_state,
				throwing_strategy );

			if( res )
				return res;

			AGENT * casted_agent = 0;
			// Проверяем чтобы агент был агентом заданного типа.
			res = agent_convertable_to< AGENT >(
				&m_agent,
				casted_agent,
				throwing_strategy );

			if( res )
				return res;

			event_handler_caller_ref_t event_handler_caller_ref(
				new not_null_data_real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					*casted_agent,
					m_state ) );

			return create_event_subscription(
				quick_typeid_t< MESSAGE >::m_type_wrapper,
				m_mbox_ref,
				event_handler_caller_ref,
				throwing_strategy );
		}

	private:
		//! Внутренний метод создания подписчика.
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

		//! Агент, который подписывается.
		agent_t & m_agent;
		//! Ссылка на mbox к которому делается подписка.
		mbox_ref_t m_mbox_ref;
		//! Состояние в котором хочется получать сообщения.
		const state_t * m_state;
};

//
// subscription_unbind_t
//

/*!
	Класс для изъятия подписки агента на сообщения от mbox.
*/
class SO_5_TYPE subscription_unbind_t
{
	public:
		subscription_unbind_t(
			agent_t & agent,
			const mbox_ref_t & mbox_ref );

		~subscription_unbind_t();

		//! Определить состояние в котором
		//! должно произойти событие.
		subscription_unbind_t &
		in(
			//! Состояние агента в котором была возможна обработка сообщения.
			const state_t & state );

		template< class MESSAGE, class AGENT >
		ret_code_t
		event(
			//! Обработчик события.
			void (AGENT::*pfn)( const event_data_t< MESSAGE > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			// Проверяем чтобы агент являлся владельцем состояния.
			ret_code_t res = agent_owns_state(
				m_agent,
				m_state,
				throwing_strategy );

			if( res )
				return res;

			AGENT * casted_agent = nullptr;
			// Проверяем чтобы агент был агентом заданного типа.
			res = agent_convertable_to< AGENT >(
				&m_agent,
				casted_agent,
				throwing_strategy );

			if( res )
				return res;

			type_wrapper_t type_wrapper( typeid( MESSAGE ) );

			event_handler_caller_ref_t event_handler_caller_ref(
				new real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					*casted_agent,
					m_state ) );

			return destroy_event_subscription(
				quick_typeid_t< MESSAGE >::m_type_wrapper,
				m_mbox_ref,
				event_handler_caller_ref,
				throwing_strategy );
		}

		template< class MESSAGE, class AGENT >
		ret_code_t
		event(
			//! Обработчик события.
			void (AGENT::*pfn)( const not_null_event_data_t< MESSAGE > & ),
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy = THROW_ON_ERROR )
		{
			// Проверяем чтобы агент являлся владельцем состояния.
			ret_code_t res = agent_owns_state(
				m_agent,
				m_state,
				throwing_strategy );

			if( res )
				return res;

			AGENT * casted_agent = 0;
			// Проверяем чтобы агент был агентом заданного типа.
			res = agent_convertable_to< AGENT >(
				&m_agent,
				casted_agent,
				throwing_strategy );

			if( res )
				return res;

			type_wrapper_t type_wrapper( typeid( MESSAGE ) );

			event_handler_caller_ref_t event_handler_caller_ref(
				new not_null_data_real_event_handler_caller_t< MESSAGE, AGENT >(
					pfn,
					*casted_agent,
					m_state ) );

			return destroy_event_subscription(
				quick_typeid_t< MESSAGE >::m_type_wrapper,
				m_mbox_ref,
				event_handler_caller_ref,
				throwing_strategy );
		}

	private:
		//! Внутренний метод создания отписчика.
		ret_code_t
		destroy_event_subscription(
			//! Тип сообщения.
			const type_wrapper_t & type_wrapper,
			//! mbox.
			mbox_ref_t & mbox_ref,
			//! Вызыватель обработчика.
			const event_handler_caller_ref_t & ehc,
			//! Флаг - бросать ли исключение в случае ошибки.
			throwing_strategy_t throwing_strategy );

		//! Агент, который подписывается.
		agent_t & m_agent;
		//! Ссылка на mbox к которому делается подписка.
		mbox_ref_t m_mbox_ref;
		//! Состояние в котором хочется получать сообщения.
		const state_t * m_state;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
