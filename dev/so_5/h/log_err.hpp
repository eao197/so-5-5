/*
	SObjectizer 5
*/

/*!
	\file
	\brief Auxiliary macros
*/

#ifndef _SO_5__LOG_ERR_HPP_
#define _SO_5__LOG_ERR_HPP_

#include <ace/Log_Msg.h>

/*!
	\brief Macros to create format string for using with ACE logging.

	Usage sample:
	\code
	ACE_ERROR(( LM_ERROR, SO_5_LOG_FMT( "invalid cmd: %s" ), cmd ));
	\endcode
*/
#define SO_5_LOG_FMT(s) "[%D PID:%P TID:%t] %M -- " s " @%N:%l\n"

#endif

