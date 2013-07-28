/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс слоя SObjectizer.
*/

#if !defined( _SO_5__RT__SO_LAYER_HPP_ )
#define _SO_5__RT__SO_LAYER_HPP_

#include <memory>
#include <map>

#include <ace/Refcounted_Auto_Ptr.h>
#include <ace/Null_Mutex.h>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/type_wrapper.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

namespace impl
{

class layer_core_t;

} /* namespace impl */


//
// so_layer_t
//

//! Интерфейс слоя SObjectizer.
/*!
	Нужен для механизма дополнительных слоев  SObjectizer.

	\note Хоть контроль за работой слоя и определен в секции public, но
	при создании собственных слоев желательно его прятать
	в секцию  protected или private, чтобы исключить
	возможность вызова методов контроля слоя в пользовательском коде.
*/
class SO_5_TYPE so_layer_t
{
		friend class impl::layer_core_t;
	public:
		so_layer_t();
		virtual ~so_layer_t();


		//! \name Контроль работы слоя.
		//! \{

		//! Запустить слой.
		/*!
			Реализация по умолчанию ничего не делает.
		*/
		virtual ret_code_t
		start();

		//! Инициировать завершение выполнения слоя.
		/*!
			Реализация по умолчанию ничего не делает.
		*/
		virtual void
		shutdown();

		//! Ожидание завершения слоя.
		/*!
			Реализация по умолчанию ничего не делает.
		*/
		virtual void
		wait();
		//! \}

	protected:
		//! Получить среду SObjectizer.
		/*!
			Если слой не привязан к SObjectizer,
			то выбрасывается исключение.
		*/
		so_environment_t &
		so_environment();

	private:
		//! Привязать к среде SObjectizer.
		void
		bind_to_environment( so_environment_t * env );

		//! Cреде SObjectizer на которую наложен данный слой.
		so_environment_t * m_so_environment;
};

//! Псевдоним unique_ptr для so_layer_t.
typedef std::unique_ptr< so_layer_t > so_layer_unique_ptr_t;

//! Псевдоним разделяемого указателя на so_layer_t.
typedef std::shared_ptr< so_layer_t >
	so_layer_ref_t;

//! Псевдоним для карты типов к слоям.
typedef std::map< type_wrapper_t, so_layer_ref_t > so_layer_map_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
