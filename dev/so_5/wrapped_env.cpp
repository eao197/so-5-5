/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.9
 * \file
 * \brief Class wrapped_env and its details.
 */

#pragma once

#include <so_5/h/wrapped_env.hpp>

#include <thread>

namespace so_5 {

/*!
 * \since v.5.5.9
 * \brief Implementation details for wrapped_env.
 */
struct wrapped_env_t::details_t
	{
		//! Actual environment object.
		so_5::api::impl::so_quick_environment_t< so_5::api::generic_simple_init_t > m_env;

		//! Helper thread for calling run method.
		std::thread m_env_thread;

		//! Initializing constructor.
		details_t(
			so_5::api::generic_simple_init_t init_func,
			so_5::rt::environment_params_t && params )
			:	m_env{ std::move( init_func ), std::move( params ) }
			,	m_env_thread{ [this]{ m_env.run(); } }
			{}

		void
		stop() { m_env.stop(); }

		void
		join() { if( m_env_thread.joinable() ) m_env_thread.join(); }
	};

namespace
{

std::unique_ptr< wrapped_env_t::details_t >
make_details_object(
	so_5::api::generic_simple_init_t init_func,
	so_5::rt::environment_params_t && params )
	{
		return std::unique_ptr< wrapped_env_t::details_t >(
				new wrapped_env_t::details_t{
						std::move( init_func ),
						std::move( params )
				} );
	}

so_5::rt::environment_params_t
make_necessary_tuning( so_5::rt::environment_params_t && params )
	{
		params.disable_autoshutdown();
		return std::move( params );
	}

so_5::rt::environment_params_t
make_params_via_tuner( so_5::api::generic_simple_so_env_params_tuner_t tuner )
	{
		so_5::rt::environment_params_t params;
		tuner( params );
		return std::move( params );
	}

} /* namespace anonymous */

wrapped_env_t::wrapped_env_t()
	:	m_impl{
			make_details_object(
					[]( so_5::rt::environment_t & ) {},
					make_necessary_tuning( so_5::rt::environment_params_t{} ) )
		}
	{}

wrapped_env_t::wrapped_env_t(
	so_5::api::generic_simple_init_t init_func )
	:	m_impl{
			make_details_object(
					std::move( init_func ),
					make_necessary_tuning( so_5::rt::environment_params_t{} ) )
		}
	{}

wrapped_env_t::wrapped_env_t(
	so_5::api::generic_simple_init_t init_func,
	so_5::api::generic_simple_so_env_params_tuner_t params_tuner )
	:	m_impl{
			make_details_object(
					std::move( init_func ),
					make_necessary_tuning( 
						make_params_via_tuner( std::move( params_tuner ) ) ) )
		}
	{}

wrapped_env_t::wrapped_env_t(
	so_5::api::generic_simple_init_t init_func,
	so_5::rt::environment_params_t && params )
	:	m_impl{
			make_details_object(
					std::move( init_func ),
					make_necessary_tuning( std::move( params ) ) )
		}
	{}

wrapped_env_t::~wrapped_env_t()
	{
		stop_then_join();
	}

so_5::rt::environment_t &
wrapped_env_t::environment() const
	{
		return m_impl->m_env;
	}

void
wrapped_env_t::stop()
	{
		m_impl->stop();
	}

void
wrapped_env_t::join()
	{
		m_impl->join();
	}

void
wrapped_env_t::stop_then_join()
	{
		stop();
		join();
	}

} /* namespace so_5 */


