/*
 * Auxilary function for simple extraction of milliseconds
 * value from ACE_Time_Value.
 *
 * It is here because of difference of ACE_Time_Value::msec()
 * under VisualStudio 2005 and 2003.
 */
#if !defined( TEST__BENCH__TIME_VALUE_MSEC_HELPER_HPP )
#define TEST__BENCH__TIME_VALUE_MSEC_HELPER_HPP

#include <ace/Time_Value.h>

inline ACE_UINT64
milliseconds( const ACE_Time_Value & tv )
	{
		ACE_UINT64 v;
		tv.msec( v );
		return v;
	}

#endif

