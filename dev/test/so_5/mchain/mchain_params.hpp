#pragma once

#include <vector>
#include <string>
#include <utility>

#include <so_5/all.hpp>

using namespace std;

vector< pair< string, so_5::rt::mchain_params_t > >
build_mchain_params()
	{
		namespace props = so_5::rt::mchain_props;

		vector< pair< string, so_5::rt::mchain_params_t > > params;
		params.emplace_back( "unlimited",
				so_5::rt::mchain_params_t{
						props::capacity_t::make_unlimited() } );
		params.emplace_back( "limited(dynamic,nowait)",
				so_5::rt::mchain_params_t{
						props::capacity_t::make_limited_without_waiting(
								5,
								props::storage_memory::dynamic,
								props::overflow_reaction::drop_newest ) } );
		params.emplace_back( "limited(preallocated,nowait)",
				so_5::rt::mchain_params_t{
						props::capacity_t::make_limited_without_waiting(
								5,
								props::storage_memory::preallocated,
								props::overflow_reaction::drop_newest ) } );
		params.emplace_back( "limited(dynamic,wait)",
				so_5::rt::mchain_params_t{
						props::capacity_t::make_limited_with_waiting(
								5,
								props::storage_memory::dynamic,
								props::overflow_reaction::drop_newest,
								chrono::milliseconds(200) ) } );
		params.emplace_back( "limited(preallocated,wait)",
				so_5::rt::mchain_params_t{
						props::capacity_t::make_limited_with_waiting(
								5,
								props::storage_memory::preallocated,
								props::overflow_reaction::drop_newest,
								chrono::milliseconds(200) ) } );
		return params;
	}

