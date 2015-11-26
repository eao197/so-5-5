#pragma once

#include <vector>
#include <string>
#include <utility>

#include <so_5/all.hpp>

using namespace std;

vector< pair< string, so_5::rt::bag_params_t > >
build_bag_params()
	{
		namespace bag_ns = so_5::rt::msg_bag;

		vector< pair< string, so_5::rt::bag_params_t > > params;
		params.emplace_back( "unlimited",
				so_5::rt::bag_params_t{
						bag_ns::capacity_t::make_unlimited() } );
		params.emplace_back( "limited(dynamic,nowait)",
				so_5::rt::bag_params_t{
						bag_ns::capacity_t::make_limited_without_waiting(
								5,
								bag_ns::storage_memory::dynamic,
								bag_ns::overflow_reaction::drop_newest ) } );
		params.emplace_back( "limited(preallocated,nowait)",
				so_5::rt::bag_params_t{
						bag_ns::capacity_t::make_limited_without_waiting(
								5,
								bag_ns::storage_memory::preallocated,
								bag_ns::overflow_reaction::drop_newest ) } );
		params.emplace_back( "limited(dynamic,wait)",
				so_5::rt::bag_params_t{
						bag_ns::capacity_t::make_limited_with_waiting(
								5,
								bag_ns::storage_memory::dynamic,
								bag_ns::overflow_reaction::drop_newest,
								chrono::milliseconds(200) ) } );
		params.emplace_back( "limited(preallocated,wait)",
				so_5::rt::bag_params_t{
						bag_ns::capacity_t::make_limited_with_waiting(
								5,
								bag_ns::storage_memory::preallocated,
								bag_ns::overflow_reaction::drop_newest,
								chrono::milliseconds(200) ) } );
		return params;
	}

