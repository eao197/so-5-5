/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Среда исполнения so_5.
*/

#if !defined( _SO_5__RT__AGENT_COOP_HPP_ )
#define _SO_5__RT__AGENT_COOP_HPP_

#include <vector>
#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>
#include <so_5/h/types.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/nonempty_name.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class agent_core_t;
class so_environment_impl_t;

} /* namespace impl */


class so_environment_t;
class agent_coop_t;

//! Псевдоним для agent_coop_t.
typedef std::unique_ptr< agent_coop_t > agent_coop_unique_ptr_t;

//! Кооперация агентов.
/*!
	Служит для объединения несольких агентов, как одной рабочей единицы
	SObjectizer. При регистрации кооперации все агенты должны быть
	зарегистрированы, иначе регистрация кооперации не состоиться.
	Агенты добавляются в кооперацию через метод кооперации add_agent().
	При добавлении агента в кооперацию ей передается владение этим агентом.
*/
class SO_5_TYPE agent_coop_t
{
		friend class agent_t;
		friend class impl::agent_core_t;

		//! Конструктор кооперации.
		agent_coop_t(
			//! Имя кооперации.
			const nonempty_name_t & name,
			//! Привязка к диспетчеру для агентов
			//! коопераций по-умолчанию.
			disp_binder_unique_ptr_t & coop_disp_binder,
			//! Реализация среды so_5.
			impl::so_environment_impl_t & env_impl );

	public:
		//! Создание кооперации.
		static agent_coop_unique_ptr_t
		create_coop(
			//! Имя кооперации.
			const nonempty_name_t & name,
			//! Привязка к диспетчеру для агентов
			//! коопераций по-умолчанию.
			disp_binder_unique_ptr_t & coop_disp_binder,
			//! Реализация среды so_5 в рамках которой
			//! создается кооперация.
			impl::so_environment_impl_t & env_impl );

		virtual ~agent_coop_t();

		//! Получить имя кооперации.
		const std::string &
		query_coop_name() const;

		//! Добавить агента в кооперацию,
		//! владение агентом переходит к gent_coop_t.
		ret_code_t
		add_agent(
			//! Агент.
			const agent_ref_t & agent_ref );

		//! Добавить агента в кооперацию с указанием привязки
		//! к конкретному дисптчеру,
		//! владение агентом переходит к agent_coop_t.
		/*!
			Если агент, добавляемый в кооперацию, должен иметь
			привязку к диспетчеру отличную от привязки
			всей кооперации, то используется этот метод,
			параметр \a disp_binder которого определяет
			специфичного для данного агента привязывателя.
		*/
		ret_code_t
		add_agent(
			//! Агент.
			const agent_ref_t & agent_ref,
			//! Объект для привязки к диспетчеру.
			disp_binder_unique_ptr_t disp_binder );

		static inline void
		call_agent_finished( agent_coop_t & coop )
		{
			coop.agent_finished();
		}

		//! Выполнить окончательную дерегистрацию кооперации.
		static inline void
		call_final_deregister_coop( agent_coop_t * coop )
		{
			coop->final_deregister_coop();
		}

	private:
		//! Агент и его привязка.
		/*!
			Элемент списка в котором хранятся агенты и их привязыватели.
		*/
		struct agent_with_disp_binder_t
		{
			agent_with_disp_binder_t(
				//! Агент.
				const agent_ref_t & agent_ref,
				//! Привязка.
				const disp_binder_ref_t & binder )
				:
					m_agent_ref( agent_ref ),
					m_binder( binder )
			{}

			//! Агент.
			agent_ref_t m_agent_ref;

			//! Привязка.
			disp_binder_ref_t m_binder;
		};

		//! Тип для массива агентов c их привязками к диспетчерам.
		typedef std::vector< agent_with_disp_binder_t > agent_array_t;

		//! Привязать агентов к кооперации.
		void
		bind_agents_to_coop();

		//! Вызвать у всех агентов метод so_define_agent().
		void
		define_all_agents();

		//! Вызвать у всех агентов метод undefine_agent().
		void
		undefine_all_agents();

		//! Вызвать у агентов undefine_agent().
		void
		undefine_some_agents(
			//! Итератор на агента у всех предшественников
			//! которого надо вызывать undefine_agent().
			agent_array_t::iterator it );

		//! Выполнить привязку агентов к диспетчерам.
		void
		bind_agents_to_disp();

		//! Отвязать агентов от диспетчеров.
		void
		unbind_agents_to_disp(
			//! Итератор на агента всех предшественников
			//! которого надо отвязать от лиспетчеров.
			agent_array_t::iterator it );

		//! Отметить, что очередной агент кооперации, завершил работу.
		/*!
			После дерегистрации кооперации, ее агенты,
			продолжают обрабатывать события, которые успели
			встать в очередь до факта дерегистрации, а когда
			агент видит, что все события, которые ему надо было
			обработать - обработаны, агент говорит собственной
			кооперации, что он закончил.
			Когда все агенты кооперации завершают свою работу,
			кооперацию можно уничтожать.
		*/
		void
		agent_finished();

		//! Выполнить окончательную дерегистрацию кооперации.
		void
		final_deregister_coop();

		//! Имя кооперации.
		const std::string m_coop_name;

		//! Мутекс для синхронизации операций над кооперацией.
		ACE_Thread_Mutex & m_lock;

		//! Флаг того, что агенты кооперации разопределены.
		bool m_agents_are_undefined;

		//! Привязка к диспетчеру для агентов коопераций по-умолчанию.
		disp_binder_ref_t m_coop_disp_binder;

		//! Массив агентов кооперации.
		agent_array_t m_agent_array;

		//! Реализация среды so_5.
		impl::so_environment_impl_t & m_so_environment_impl;

		//! Количество работающих агентов.
		atomic_counter_t m_working_agents_count;
};

//! Тип для умного указателя на agent_coop_t.
typedef std::shared_ptr< agent_coop_t > agent_coop_ref_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
