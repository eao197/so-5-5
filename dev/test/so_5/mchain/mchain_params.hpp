#pragma once

#include <vector>
#include <string>
#include <utility>

#include <so_5/all.hpp>

using namespace std;

vector< pair< string, so_5::mchain_params > >
build_mchain_params()
	{
		namespace props = so_5::mchain_props;

		vector< pair< string, so_5::mchain_params > > params;
		params.emplace_back( "unlimited",
				so_5::make_unlimited_mchain_params() );
		params.emplace_back( "limited(dynamic,nowait)",
				so_5::make_limited_without_waiting_mchain_params(
						5,
						props::storage_memory::dynamic,
						props::overflow_reaction::drop_newest ) );
		params.emplace_back( "limited(preallocated,nowait)",
				so_5::make_limited_without_waiting_mchain_params(
						5,
						props::storage_memory::preallocated,
						props::overflow_reaction::drop_newest ) );
		params.emplace_back( "limited(dynamic,wait)",
				so_5::make_limited_with_waiting_mchain_params(
						5,
						props::storage_memory::dynamic,
						props::overflow_reaction::drop_newest,
						chrono::milliseconds(200) ) );
		params.emplace_back( "limited(preallocated,wait)",
				so_5::make_limited_with_waiting_mchain_params(
						5,
						props::storage_memory::preallocated,
						props::overflow_reaction::drop_newest,
						chrono::milliseconds(200) ) );

		return params;
	}

