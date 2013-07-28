/*
	SObjectizer 5 Transport.
*/

#if !defined( _SO_5_TRANSPORT__DECLSPEC_HPP_ )
#define _SO_5_TRANSPORT__DECLSPEC_HPP_

#include <cpp_util_2/h/detect_compiler.hpp>

#if defined( SO_5_TRANSPORT_PRJ )
	#define SO_5_TRANSPORT_TYPE\
	CPP_UTIL_2_EXPORT

	#define SO_5_TRANSPORT_EXPORT_FUNC_SPEC(ret_type)\
	CPP_UTIL_2_EXPORT_FUNC_SPEC(ret_type)

#else
	#define SO_5_TRANSPORT_TYPE\
	CPP_UTIL_2_IMPORT

	#define SO_5_TRANSPORT_EXPORT_FUNC_SPEC(ret_type)\
	CPP_UTIL_2_IMPORT_FUNC_SPEC(ret_type)

#endif

#endif
