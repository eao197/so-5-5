/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.4
 * \brief Interfaces of data source and data sources repository.
 */

#include <so_5/rt/stats/h/repository.hpp>

namespace so_5
{

namespace rt
{

namespace stats
{

//
// source_t
//
source_t::source_t()
	:	m_prev{ nullptr }
	,	m_next{ nullptr }
	{}

source_t::~source_t()
	{}

//
// repository_t
//
repository_t::~repository_t()
	{}

} /* namespace stats */

} /* namespace rt */

} /* namespace so_5 */

