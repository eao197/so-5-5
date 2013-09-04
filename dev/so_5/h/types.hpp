/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Various typedefs.
*/

#if !defined( _SO_5__TYPES_HPP_ )
#define _SO_5__TYPES_HPP_

#include <ace/Atomic_Op.h>

namespace so_5
{

//! Atomic counter type.
typedef ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long>
	atomic_counter_t;


//! Atomic flag type.
typedef ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long>
	atomic_flag_t;
}

#endif
