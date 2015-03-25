/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.4
 * \file
 * \brief Helpers creation of data source name prefixes.
 */

#pragma once

#include <so_5/rt/stats/h/prefix.hpp>

#include <sstream>

namespace so_5 {

namespace disp {

namespace reuse {

/*!
 * \since v.5.5.4
 * \brief Create basic prefix for dispatcher data source names.
 */
inline so_5::rt::stats::prefix_t
make_disp_prefix(
	//! Textual marker of dispatcher type (like ot, ao, ag, tp, atp, ...).
	const char * disp_type,
	//! User-supplied optional base for name. Can be empty.
	const std::string data_sources_name_base,
	//! Pointer to dispatcher instance.
	//! Will be used if \a data_sources_name_base is empty.
	const void * disp_this_pointer )
	{
		const std::size_t max_name_base_fragment = 24;
		std::ostringstream ss;

		ss << "disp/" << disp_type << "/";
		if( !data_sources_name_base.empty() )
			{
				if( data_sources_name_base.size() > max_name_base_fragment )
					{
						const std::size_t median = max_name_base_fragment / 2;

						ss << data_sources_name_base.substr( 0, median )
							<< "..."
							<< data_sources_name_base.substr(
								data_sources_name_base.size() - median + 3 );
					}
				else
					ss << data_sources_name_base;
			}
		else
			{
				const char * z = nullptr;
				auto t = reinterpret_cast< const char * >(disp_this_pointer);
				ss << "0x" << std::hex << (t - z);
			}

		return so_5::rt::stats::prefix_t{ ss.str() };
	}

/*!
 * \since v.5.5.4
 * \brief Create prefix for dispatcher's working thread data source.
 */
inline so_5::rt::stats::prefix_t
make_disp_working_thread_prefix(
	//! Prefix for the dispatcher.
	const so_5::rt::stats::prefix_t & disp_prefix,
	//! Thread ordinal number.
	std::size_t thread_number )
	{
		std::ostringstream ss;
		ss << disp_prefix.c_str() << "/wt-" << thread_number;

		return ss.str();
	}

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */

