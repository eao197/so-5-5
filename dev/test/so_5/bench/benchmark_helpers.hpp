/*
 * SObjectizer-5
 */
/*!
 * \since v.5.4.0
 * \file
 * \brief A simple helpers for building benchmarks.
 */
#pragma once

#include <chrono>
#include <iostream>

//! A helper for fixing starting and finishing time points and
//! calculate events processing time and events throughtput.
class benchmarker_t
	{
	public :
		//! Fix starting time.
		inline void
		start()
			{
				m_start = std::chrono::high_resolution_clock::now();
			}

		//! Fix finish time and show stats.
		inline void
		finish_and_show_stats(
			unsigned long long events,
			const std::string & title )
			{
				auto finish_time = std::chrono::high_resolution_clock::now();
				const double duration =
						std::chrono::duration_cast< std::chrono::milliseconds >(
								finish_time - m_start ).count() / 1000.0;
				const double price = duration / events;
				const double throughtput = 1 / price;

				std::cout.precision( 10 );
				std::cout << title << ": " << events
						<< ", total_time: " << duration << "s"
						<< ", price: " << price << "s"
						<< ", throughtput: " << throughtput << " " << title << "/s"
						<< std::endl;
			}

	private :
		std::chrono::high_resolution_clock::time_point m_start;
	};

