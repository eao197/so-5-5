/*
	SObjectizer 5
*/

/*!
	\file
	\brief Вспомогательные макросы.
*/

#ifndef _SO_5__LOG_ERR_HPP_
#define _SO_5__LOG_ERR_HPP_

#include <iostream>
#include <sstream>

#include <ace/Log_Msg.h>

/*!
	\brief Макрос для создания форматной строки для вывода
	сообщений через ACE logging.

	Пример использования:
	\code
	ACE_ERROR(( LM_ERROR, SO_5_LOG_FMT( "invalid cmd: %s" ), cmd ));
	\endcode
*/
#define SO_5_LOG_FMT(s) "[%D PID:%P TID:%t] %M -- " s " @%N:%l\n"

/*!
 * \brief Макрос для контроля кода возврата ACE-функции и
 * выполнения abort в случае необходимости.
 *
 * Предназначен для прерывания программы в случае, когда неудачно
 * завершается операция, которая не должна так делать. И когда
 * восстановление после подобной ситуации невозможно (например,
 * не удалось стартовать рабочую нить).
 *
 * Пример использования:
 * \code
 * SO_5_ABORT_ON_ACE_ERROR(
 * 	ACE_Thread_Manager::instance()->spawn( *параметры* ) );
 * \endcode
 */
#define SO_5_ABORT_ON_ACE_ERROR(action) do { \
	if( -1 == (action) ) {\
		ACE_ERROR((LM_EMERGENCY, SO_5_LOG_FMT( #action ))); \
		ACE_OS::abort(); \
	} \
} while(false)

#endif
