/*
	SObjectizer 5.
*/

#include <so_5/api/h/api.hpp>
#include <so_5/rt/impl/h/so_environment_impl.hpp>
#include <so_5/rt/h/so_environment.hpp>

namespace so_5
{

namespace api
{

namespace
{

//! Вспомогательный класс для запуска SObjectizer.
/*!
	Служит оберткой над функцией инициализации
	SObjectizer.
*/
class so_quick_environment_t
	:
		public so_5::rt::so_environment_t
{
		typedef so_5::rt::so_environment_t base_type_t;

	public:
		so_quick_environment_t(
			//! Указатель на инициализирующую функцию.
			pfn_so_environment_init_t init_func,
			//! Параметры создания SObjectizer.
			const so_5::rt::so_environment_params_t & env_params )
			:
				base_type_t( env_params ),
				m_init_func( init_func )
		{}
		virtual ~so_quick_environment_t()
		{}

		virtual void
		init()
		{
			(*m_init_func)( *this );
		}

	private:
		pfn_so_environment_init_t m_init_func;
};

} /* namespace ananymous */

SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	pfn_so_environment_init_t init_func,
	const so_5::rt::so_environment_params_t & env_params,
	throwing_strategy_t throwing_strategy )
{
	so_quick_environment_t env( init_func, env_params );
	return env.run( throwing_strategy );
}

SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	pfn_so_environment_init_t init_func,
	throwing_strategy_t throwing_strategy )
{
	return run_so_environment(
		init_func,
		so_5::rt::so_environment_params_t(),
		throwing_strategy );
}

//
// env_init_caller_base_t
//

env_init_caller_base_t::~env_init_caller_base_t()
{
}

namespace
{

//! Вспомогательный класс для запуска SObjectizer.
/*!
	Служит оберткой над инициализатором env_init_caller_base_t.
*/
class so_quick_paramed_environment_t
	:
		public so_5::rt::so_environment_t
{
		typedef so_5::rt::so_environment_t base_type_t;

	public:
		so_quick_paramed_environment_t(
			//! Объект который инициализирует SObjectizer.
			env_init_caller_base_t & init_caller,
			//! Параметры SObjectizer.
			const so_5::rt::so_environment_params_t & env_params )
			:
				base_type_t( env_params ),
				m_init_caller( init_caller )
		{}
		virtual ~so_quick_paramed_environment_t()
		{}

		virtual void
		init()
		{
			m_init_caller.call( *this );
		}

	private:
		env_init_caller_base_t & m_init_caller;
};

} /* namespace ananymous */

SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	env_init_caller_base_t & init_caller,
	throwing_strategy_t throwing_strategy )
{
	return run_so_environment(
		init_caller,
		so_5::rt::so_environment_params_t(),
		throwing_strategy );
}

SO_5_EXPORT_FUNC_SPEC( so_5::ret_code_t )
run_so_environment(
	env_init_caller_base_t & init_caller,
	const so_5::rt::so_environment_params_t & env_params,
	throwing_strategy_t throwing_strategy )
{
	so_quick_paramed_environment_t env( init_caller, env_params );
	return env.run( throwing_strategy );
}

} /* namespace api */

} /* namespace so_5 */
