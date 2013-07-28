/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Функция для создания диспетчера активных групп.
*/

#if !defined( _SO_5__DISP__ACTIVE_GROUP__PUB_HPP_ )
#define _SO_5__DISP__ACTIVE_GROUP__PUB_HPP_

#include <string>

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/disp.hpp>
#include <so_5/rt/h/disp_binder.hpp>

namespace so_5
{

namespace disp
{

namespace active_group
{

//! Создание диспетчера.
SO_5_EXPORT_FUNC_SPEC( so_5::rt::dispatcher_unique_ptr_t )
create_disp();

//! Получить объект для привязки агента к диспетчеру.
SO_5_EXPORT_FUNC_SPEC( so_5::rt::disp_binder_unique_ptr_t )
create_disp_binder(
	//! Имя диспетчера.
	const std::string & disp_name,
	//! Имя группы.
	const std::string & group_name );

} /* namespace active_group */

} /* namespace disp */

} /* namespace so_5 */

#endif
