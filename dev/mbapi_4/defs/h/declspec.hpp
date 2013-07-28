/*
	MBAPI 4.
*/

#if !defined( _MBAPI_4__DEFS__DECLSPEC_HPP_ )
#define _MBAPI_4__DEFS__DECLSPEC_HPP_

#include <cpp_util_2/h/detect_compiler.hpp>

#if defined( MBAPI_4_DEFS_PRJ )
	#define MBAPI_4_DEFS_TYPE \
	CPP_UTIL_2_EXPORT

	#define MBAPI_4_DEFS_EXPORT_FUNC_SPEC( ret_type ) \
	CPP_UTIL_2_EXPORT_FUNC_SPEC( ret_type )

#else
	#define MBAPI_4_DEFS_TYPE \
	CPP_UTIL_2_IMPORT

	#define MBAPI_4_DEFS_EXPORT_FUNC_SPEC( ret_type ) \
	CPP_UTIL_2_IMPORT_FUNC_SPEC( ret_type )

#endif

#endif
