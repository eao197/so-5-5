/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.8
 * \brief Definition of priority type.
 */

#pragma once

namespace so_5 {

/*!
 * \since v.5.5.8
 * \brief Definition of supported priorities.
 */
enum class priority_t : unsigned char
	{
		p_min = 0,
		p0 = p_min,
		p1,
		p2,
		p3,
		p4,
		p5,
		p6,
		p7,
		p_max = p7
	};

inline std::size_t
to_size_t( priority_t priority )
	{
		return static_cast< std::size_t >( priority );
	}

namespace prio {

/*!
 * \name Constants with priority values.
 * \{
 */
const priority_t p0 = priority_t::p0;
const priority_t p1 = priority_t::p1;
const priority_t p2 = priority_t::p2;
const priority_t p3 = priority_t::p3;
const priority_t p4 = priority_t::p4;
const priority_t p5 = priority_t::p5;
const priority_t p6 = priority_t::p6;
const priority_t p7 = priority_t::p7;
/*!
 * \}
 */

} /* namespace prio */

} /* namespace so_5 */

