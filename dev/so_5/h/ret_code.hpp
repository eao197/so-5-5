/*
	SObjectizer 5
*/

/*!
	\file
	\brief Коды ошибок.
*/

#if !defined( _SO_5__RET_CODE_HPP_ )
#define _SO_5__RET_CODE_HPP_

#include <so_5/h/declspec.hpp>

#include <iostream>

namespace so_5
{

//! Код возврата основных API-функций и методов.
typedef int ret_code_t;

//
// Стандартные коды ошибок
//

//! Ошибка запуска so_environment.
const int rc_environment_error = 1;

//! \name Ошибки методов агента.
//! \{

//! Переход к неизвестному состоянию агента.
const int rc_agent_unknown_state = 10;

//! Агент уже привязан к среде SO.
const int rc_agent_is_already_bind_to_env = 11;

//! Агент уже привязан к диспетчеру.
const int rc_agent_is_already_bind_to_disp = 12;

//! Диспетчер с заданным именем не найден.
const int rc_named_disp_not_found = 13;

//! Агент не привязан к кооперации.
const int rc_agent_has_no_cooperation = 14;

//! Агент не может быть преведен к заданному типу.
const int rc_agent_incompatible_type_conversion = 15;

//! \}

//! Ошибки кооперации агентов.
//! \{

//! Передан нулевой указатель.
const int rc_zero_ptr_to_coop = 20;

//! Кооперация с таким именем уже зарегистрирована.
const int rc_coop_with_specified_name_is_already_registered = 21;

//! Кооперация содержит пустые ссылки на агент или привязку к диспетчеру.
const int rc_coop_has_references_to_null_agents_or_binders = 22;

//! Дерегистрируемая кооперация с заданным именем не найдена.
const int rc_coop_has_not_found_among_registered_coop = 23;

//! Кооперация не мождет быть зарегистрирована.
const int rc_coop_define_agent_failed = 24;

//! \}


//! \name Ошибки диспетчера.
//! \{

//! Не удалось запустить диспетчер.
const int rc_disp_start_failed = 30;

//! Не удалось создать диспетчер.
const int rc_disp_create_failed = 31;

//! Привязка агента и его реальный тип не совпадают.
const int rc_disp_type_mismatch = 32;

//! \}

//! \name Ошибки регистрации обработчиков
//! событий и перехватчиков сообщений.
//! \{

//! Регистрация обработчика события на тип сообщения, mbox и
//! состояние для которых уже зарегистрирован обработчик.
const int rc_evt_handler_already_provided = 40;

//! Дерегистрация обработчика на тип сообщения, mbox и
//! состояние, которого нет в списке зарегистрированных.
const int rc_no_event_handler_provided = 41;

//! Деригистрация обработчика на тип сообщения, mbox и
//! состояние, которого есть в списке состояний, но сам обработчик
//! другой нежели указанный в параметрах.
const int rc_event_handler_match_error = 42;

//! Агент не является владельцем состояния,
//! которое указано при регистрации.
const int rc_agent_is_not_the_state_owner = 43;

//! Регистрация перехватчика события на тип сообщения, mbox и
//! состояние для которых уже зарегистрирован обработчик.
const int rc_intercept_handler_already_provided = 44;

//! Деригистрация перехватчика на тип сообщения, mbox и
//! состояние, которого нет в списке зарегистрированных.
const int rc_no_interception_handler_provided = 45;

//! Деригистрация перехватчика на тип сообщения, mbox и
//! состояние, которого есть в списке состояний, но сам обработчик
//! другой нежели указанный в параметрах.
const int rc_interception_handler_match_error = 46;
//! \}

//! \name Ошибки mbox-ов.
//! \{

//! Регистрация именованного mbox-а с именем, которое уже есть.
const int rc_mbox_duplicating_name = 80;

//! Не удается найти именованный mbox.
const int rc_mbox_unable_to_find_mbox = 81;
//! \}

//! \name Ошибки работы отложенных или периодических сообщений.
//! \{

//! Не удается запланировать таймерное событие.
const int rc_unable_to_schedule_timer_act = 90;
//! \}

//! \name Ошибки работы cо слоем.
//! \{

//! Слой не привязан к среде SObjectizer.
const int rc_layer_not_binded_to_so_env = 100;

//! Попытка добавить дополнительный слой
//! по нулевому указателю.
const int rc_trying_to_add_nullptr_extra_layer = 101;

//! Попытка добавить дополнительный слой,
//! который уже находиться в списке слоев по умолчанию.
const int rc_trying_to_add_extra_layer_that_already_exists_in_default_list = 102;

//! Попытка добавить дополнительный слой,
//! который уже находиться в списке дополнительных слоев.
const int rc_trying_to_add_extra_layer_that_already_exists_in_extra_list = 103;

//! Попытка инициализировать слой прошла неудачно.
const int rc_unable_to_start_extra_layer = 104;

//! Слой заданного типа не существует.
const int rc_layer_does_not_exist = 105;
//! \}

//! \name Ошибки общего плана.
//! \{

//! Пустое имя ( mbox-а, кооперации ).
const int rc_empty_name = 500;

const int rc_unexpected_error = 0xFFFFFF;
//! \}

} /* namespace so_5 */

#endif
