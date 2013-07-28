/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Различные псевдонимы типов.
*/

#if !defined( _SO_5__TYPES_HPP_ )
#define _SO_5__TYPES_HPP_

#include <ace/Atomic_Op.h>

namespace so_5
{

//! Тип атамарного счетчика.
typedef ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long>
	atomic_counter_t;


//! Тип атамарного флага.
typedef ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long>
	atomic_flag_t;
}

#endif
