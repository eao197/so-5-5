/*
	SObjectizer 5.
*/

/*!
	\file
	\brief  Функция для создания диспетчера с одной рабочей нитью.
*/

#if !defined( _SO_5__DISP__ONE_THREAD__PUB_HPP_ )
#define _SO_5__DISP__ONE_THREAD__PUB_HPP_

#include <string>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace disp
{

namespace one_thread
{

//! Создание диспетчера.
SO_5_EXPORT_FUNC_SPEC( so_5::rt::dispatcher_unique_ptr_t )
create_disp();

//! Получить объект для привязки агента к диспетчеру.
SO_5_EXPORT_FUNC_SPEC( so_5::rt::disp_binder_unique_ptr_t )
create_disp_binder(
	//! Имя диспетчера.
	const std::string & disp_name );

} /* namespace one_thread */

} /* namespace disp */

} /* namespace so_5 */


#endif
