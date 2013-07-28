/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Инерфейс слущателя коопераций.
*/

#if !defined( _SO_5__RT__COOP_LISTENER_HPP_ )
#define _SO_5__RT__COOP_LISTENER_HPP_

#include <string>
#include <memory>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

//
// coop_listener_t
//

//! Слушатель действий происходящих с кооперациями.
/*!
	Служит для обработки случаев регистрации
	и дерегистрации коопераций.

	Вызовы обработчиков не синхронизируются,
	и могут происходить с разных нитей. Поэтому в случае
	необходимости синхронизации о ней должен позаботиться
	прикладной программист.
*/
class SO_5_TYPE coop_listener_t
{
	public:
		virtual ~coop_listener_t();

		//! Метод обработки регистрации кооперации.
		/*!
			Вызывается после того, как кооперация успешно зарегистрирована.
		*/
		virtual void
		on_registered(
			//! Среда Sobjectizer.
			so_environment_t & so_env,
			//! Имя зарегистрированной кооперации.
			const std::string & coop_name ) throw() = 0;

		//! Метод обработки дерегистрации кооперации.
		/*!
			Вызывается после того, как кооперация окончательно дерегистрирована.
		*/
		virtual void
		on_deregistered(
			//! Среда Sobjectizer.
			so_environment_t & so_env,
			//! Имя дерегистрированной кооперации.
			const std::string & coop_name ) throw() = 0;
};

//! Псевдоним unique_ptr для coop_listener_t.
typedef std::unique_ptr< coop_listener_t > coop_listener_unique_ptr_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
