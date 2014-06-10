/*
	SObjectizer 5.
*/

/*!
 * \since v.5.3.0
 * \file 
 * \brief Detection of compiler version and absence of various features.
 */

#if !defined( SO_5__COMPILER_FEATURES_HPP )
#define SO_5__COMPILER_FEATURES_HPP

#if defined( _MSC_VER ) && ( _MSC_VER <= 1700 )
	// Visual C++ 2012 and earlier doesn't support variadic templates.
	#define SO_5_NO_VARIADIC_TEMPLATES
#endif 

#endif

