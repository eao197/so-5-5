/*
 * A tests for layers.
 */

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <exception>

#include <cpp_util_2/h/defs.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/rt/h/rt.hpp>
#include <so_5/rt/h/so_layer.hpp>

#include <so_5/api/h/api.hpp>

#include <utest_helper_1/h/helper.hpp>

#include "../separate_so_thread_inl.cpp"

template < int N >
class test_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:
		test_layer_t()
		{}

		virtual ~test_layer_t()
		{}

		virtual so_5::ret_code_t
		start()
		{
			return 0;
		}

		virtual void
		shutdown()
		{}

		virtual void
		wait()
		{}
};

class test_layer_bad_start_t
	:
		public so_5::rt::so_layer_t
{
	public:
		test_layer_bad_start_t()
		{}

		virtual ~test_layer_bad_start_t()
		{}

		virtual so_5::ret_code_t
		start()
		{
			return 2013;
		}

		virtual void
		shutdown()
		{}

		virtual void
		wait()
		{}
};

class so_environment_error_checker_t
	:	public so_5::rt::so_environment_t
	,	public separate_so_thread::init_finish_signal_mixin_t
{
		typedef so_5::rt::so_environment_t base_type_t;
	public:
		so_environment_error_checker_t()
			:
				base_type_t(
					so_5::rt::so_environment_params_t()
						.mbox_mutex_pool_size( 4 )
						.agent_event_queue_mutex_pool_size( 4 )
						.add_layer( std::unique_ptr< test_layer_t< 0 > >(
							new test_layer_t< 0 > ) ) )
		{}

		virtual ~so_environment_error_checker_t(){}

		virtual void
		init()
		{
			init_finished();
		}
};

UT_UNIT_TEST( check_errors )
{
	so_environment_error_checker_t so_env;

	separate_so_thread::run_on( so_env, [&]() {
			// Try to set up layer which is already set.
			UT_CHECK_EQ(
				so_5::rc_trying_to_add_extra_layer_that_already_exists_in_default_list,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 0 > >( new test_layer_t< 0 > ),
					so_5::DO_NOT_THROW_ON_ERROR ) );

			// Try to set up layer by zero pointer.
			UT_CHECK_EQ(
				so_5::rc_trying_to_add_nullptr_extra_layer,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 1 > >( nullptr ),
					so_5::DO_NOT_THROW_ON_ERROR ) );

			// Try to add new layer. No errors expected.
			UT_CHECK_EQ(
				0,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 1 > >( new test_layer_t< 1 > ),
					so_5::DO_NOT_THROW_ON_ERROR ) );

			// Try to add layer which is already set.
			UT_CHECK_EQ(
				so_5::rc_trying_to_add_extra_layer_that_already_exists_in_extra_list,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 1 > >( new test_layer_t< 1 > ),
					so_5::DO_NOT_THROW_ON_ERROR ) );

			// Try to add layer which is failed to start.
			UT_CHECK_EQ(
				so_5::rc_unable_to_start_extra_layer,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_bad_start_t >( new test_layer_bad_start_t ),
					so_5::DO_NOT_THROW_ON_ERROR ) );
		} );
}

UT_UNIT_TEST( check_exceptions )
{
	so_environment_error_checker_t so_env;

	separate_so_thread::run_on( so_env, [&]() {
			// Try to set up layer which is already set.
			UT_CHECK_THROW(
				so_5::exception_t,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 0 > >( new test_layer_t< 0 > ) ) );

			// Try to set up layer by zero pointer.
			UT_CHECK_THROW(
				so_5::exception_t,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 1 > >( nullptr ) ) );

			// Try to add new layer. No errors expected.
			so_env.add_extra_layer(
				std::unique_ptr< test_layer_t< 1 > >( new test_layer_t< 1 > ) );

			// Try to add layer which is already set.
			UT_CHECK_THROW(
				so_5::exception_t,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_t< 1 > >( new test_layer_t< 1 > ) ) );

			// Try to add layer which is failed to start.
			UT_CHECK_THROW(
				so_5::exception_t,
				so_env.add_extra_layer(
					std::unique_ptr< test_layer_bad_start_t >( new test_layer_bad_start_t ) ) );
		} );
}

int
main( int, char ** )
{
	UT_RUN_UNIT_TEST( check_errors );
	UT_RUN_UNIT_TEST( check_exceptions );

	return 0;
}
