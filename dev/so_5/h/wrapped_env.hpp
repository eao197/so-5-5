/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.9
 * \file
 * \brief Class wrapped_env and its details.
 */

#pragma once

#include <so_5/api/h/api.hpp>

#include <so_5/h/declspec.hpp>

#include <memory>

namespace so_5 {

#if defined( SO_5_MSVC )
	#pragma warning(push)
	#pragma warning(disable: 4251)
#endif

//FIXME: an usage example must be present in the comment.
/*!
 * \since v.5.5.9
 * \brief A wrapped environment.
 */
class SO_5_TYPE wrapped_env_t
	{
	public :
		wrapped_env_t( const wrapped_env_t & ) = delete;
		wrapped_env_t( wrapped_env_t && ) = delete;

		//! Default constructor.
		/*!
		 * Starts environment without any initialization actions.
		 */
		wrapped_env_t();

		//! A constructor which receives only initialization function.
		/*!
		 * Default environment parameters will be used.
		 */
		wrapped_env_t(
			//! Initialization function.
			so_5::api::generic_simple_init_t init_func );

		//! A constructor which receives initialization function and
		//! a function for environment's params tuning.
		wrapped_env_t(
			//! Initialization function.
			so_5::api::generic_simple_init_t init_func,
			//! Function for environment's params tuning.
			so_5::api::generic_simple_so_env_params_tuner_t params_tuner );

		//! A constructor which receives initialization function and
		//! already prepared environment's params.
		wrapped_env_t(
			//! Initialization function.
			so_5::api::generic_simple_init_t init_func,
			//! Environment's params.
			so_5::rt::environment_params_t && params );

		//! Destructor.
		/*!
		 * Stops the environment and waits it.
		 */
		~wrapped_env_t();

		//! Access to wrapped environment.
		so_5::rt::environment_t &
		environment() const;

		//! Send stop signal to environment.
		void
		stop();

		//! Wait for complete finish of environment's work.
		void
		join();

		//! Send stop signal and wait for complete finish of environment's work.
		void
		stop_then_join();

		struct details_t;

	private :
		//! Implementation details.
		std::unique_ptr< details_t > m_impl;
	};

#if defined( SO_5_MSVC )
	#pragma warning(pop)
#endif

} /* namespace so_5 */

