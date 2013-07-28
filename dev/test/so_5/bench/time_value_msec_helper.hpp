/*
 * Вспомогательная функция для удобного извлечения миллисекунд
 * из ACE_Time_Value.
 *
 * Появилась из-за того, что под VisualStudio 2005 метод
 * ACE_Time_Value::msec() работает не так, как под VisualStudio 2003.
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

