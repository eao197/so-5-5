/*
 * SObjectizer-5
 * various_helpers-1
 */
/*!
 * \since v.1.0.2
 * \file
 * \brief Ensure family of helper functions.
 */

#pragma once

#include <stdexcept>
#include <string>

/*!
 * \since v.1.0.2
 * \brief Very simple form of ensure helper function.
 *
 * \par Purpose
 * Checks the \a p argument and throws an exception if \a p is not `true`.
 *
 * \par Usage sample:
 * \code
	ensure( demands_count != 0, "demands_count must be greater than 0" );
 * \endcode
 */
inline void
ensure( bool p, const char * explanation_msg )
	{
		if( !p ) throw std::runtime_error( explanation_msg );
	}

/*!
 * \since v.1.0.2
 * \brief Very simple form of ensure helper function.
 *
 * \par Purpose
 * Checks the \a p argument and throws an exception if \a p is not `true`.
 *
 * \par Usage sample:
 * \code
	ensure( demands_count <= max_demands, 
		"demands_count(" + to_string( demands_count ) + ") must be less or "
		"equal to max_demands(" + to_string( demands_count ) + ")" );
 * \endcode
 */
inline void
ensure( bool p, const std::string & explanation_msg )
	{
		if( !p ) throw std::runtime_error( explanation_msg );
	}

/*!
 * \since v.1.0.2
 * \brief Ensure helper function which allows to throw an exception of
 * the specified type.
 *
 * \par Purpose
 * Checks the \a p argument and throws an exception if \a p is not `true`.
 *
 * \par Usage sample:
 * \code
	ensure< invalid_argument >( demands_count <= max_demands, 
		"demands_count(" + to_string( demands_count ) + ") must be less or "
		"equal to max_demands(" + to_string( demands_count ) + ")" );
 * \endcode
 */
template< typename EXCEPTION, typename EXPLANATION >
inline void
ensure( bool p, EXPLANATION explanation_msg )
	{
		if( !p ) throw EXCEPTION( explanation_msg );
	}

