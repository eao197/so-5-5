/*
 * A simple test for thread_pool dispatcher.
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

#include <so_5/disp/thread_pool/h/pub.hpp>

struct msg_hello : public so_5::rt::signal_t {};

class a_test_t : public so_5::rt::agent_t
{
	public:
		a_test_t(
			so_5::rt::so_environment_t & env )
			:	so_5::rt::agent_t( env )
		{}

		virtual void
		so_define_agent()
		{
			so_subscribe( so_direct_mbox() ).event(
					so_5::signal< msg_hello >,
					&a_test_t::evt_hello );
		}

		virtual void
		so_evt_start()
		{
			so_direct_mbox()->deliver_signal< msg_hello >();
		}

		void
		evt_hello()
		{
			so_environment().stop();
		}
};

void
sobjectizer_thread( std::promise<void> * end )
	{
		try
			{
				so_5::api::run_so_environment(
					[]( so_5::rt::so_environment_t & env )
					{
						env.register_agent_as_coop(
								"test",
								new a_test_t( env ),
								so_5::disp::thread_pool::create_disp_binder(
										"thread_pool",
										so_5::disp::thread_pool::params_t() ) );
					},
					[]( so_5::rt::so_environment_params_t & params )
					{
						params.add_named_dispatcher(
								"thread_pool",
								so_5::disp::thread_pool::create_disp( 4 ) );
					} );
			}
		catch( const std::exception & x )
			{
				std::cerr << "Exception: " << x.what() << std::endl;
				std::abort();
			}

		end->set_value();
	}

int
main( int argc, char * argv[] )
{
	try
	{
		std::promise< void > end;

		std::thread so_thread( sobjectizer_thread, &end );

		auto f = end.get_future();
		f.wait_for( std::chrono::seconds(5) );

		if( !f.valid() )
			throw std::runtime_error( "SObjectizer is not finished!" );
		else
			so_thread.join();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		std::abort();
	}

	return 0;
}

