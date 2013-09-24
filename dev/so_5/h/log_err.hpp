/*
	SObjectizer 5
*/

/*!
	\file
	\brief Auxiliary macros.
*/

#ifndef _SO_5__LOG_ERR_HPP_
#define _SO_5__LOG_ERR_HPP_

#include <iostream>
#include <sstream>

#include <ace/Log_Msg.h>

/*!
	\brief Macros to create a format string for using with ACE logging.

	Usage sample:
	\code
	ACE_ERROR(( LM_ERROR, SO_5_LOG_FMT( "invalid cmd: %s" ), cmd ));
	\endcode
*/
#define SO_5_LOG_FMT(s) "[%D PID:%P TID:%t] %M -- " s " @%N:%l\n"

/*!
 * \brief Macro for the return value control and abort the program
 * if necessary.
 *
 * Should be used in cases when an error is not expected. And if
 * it is impossible to continue in the case of an error. For example if
 * attempt to add a value into the hash map is failed.
 *
 * Usage sample:
 * \code
 * SO_5_ABORT_ON_ACE_ERROR(
 * 	ACE_Thread_Manager::instance()->spawn( *params* ) );
 * \endcode
 */
#define SO_5_ABORT_ON_ACE_ERROR(action) do { \
	if( -1 == (action) ) {\
		ACE_ERROR((LM_EMERGENCY, SO_5_LOG_FMT( #action ))); \
		ACE_OS::abort(); \
	} \
} while(false)

#endif

