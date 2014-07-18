/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Various typedefs.
*/

#if !defined( _SO_5__TYPES_HPP_ )
#define _SO_5__TYPES_HPP_

#include <atomic>

namespace so_5
{

//! Atomic counter type.
typedef std::atomic_ulong atomic_counter_t;


//! Atomic flag type.
typedef std::atomic_ulong atomic_flag_t;

//! A type for mbox indentifier.
typedef unsigned long long mbox_id_t;

}

#endif
