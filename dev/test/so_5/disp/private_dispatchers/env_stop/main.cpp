/*
 * A test for calling environment_t::stop() for stopping SObjectizer.
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct msg_hello : public so_5::rt::signal_t {};

template< typename DISPATCHER_HANDLE >
void
make_coop( so_5::rt::environment_t & env, DISPATCHER_HANDLE disp )
{
	env.introduce_coop( disp->binder(),
		[]( so_5::rt::agent_coop_t & coop ) {
			auto a1 = coop.define_agent();
			auto a2 = coop.define_agent();
			auto a3 = coop.define_agent();
			auto a4 = coop.define_agent();
			auto a5 = coop.define_agent();
			auto a6 = coop.define_agent();
			auto a7 = coop.define_agent();
			auto a8 = coop.define_agent();

			a1.on_start( [a2] { so_5::send< msg_hello >( a2.direct_mbox() ); } );

			a1.event< msg_hello >( a1.direct_mbox(), [a2] {
					so_5::send< msg_hello >( a2.direct_mbox() ); } );
			a2.event< msg_hello >( a2.direct_mbox(), [a3] {
					so_5::send< msg_hello >( a3.direct_mbox() ); } );
			a3.event< msg_hello >( a3.direct_mbox(), [a4] {
					so_5::send< msg_hello >( a4.direct_mbox() ); } );
			a4.event< msg_hello >( a4.direct_mbox(), [a5] {
					so_5::send< msg_hello >( a5.direct_mbox() ); } );
			a5.event< msg_hello >( a5.direct_mbox(), [a6] {
					so_5::send< msg_hello >( a6.direct_mbox() ); } );
			a6.event< msg_hello >( a6.direct_mbox(), [a7] {
					so_5::send< msg_hello >( a7.direct_mbox() ); } );
			a7.event< msg_hello >( a7.direct_mbox(), [a1] {
					so_5::send< msg_hello >( a1.direct_mbox() ); } );
		} );
}

void
make_stopper( so_5::rt::environment_t & env )
{
	env.introduce_coop( []( so_5::rt::agent_coop_t & coop ) {
			struct msg_stop : public so_5::rt::signal_t {};

			auto a1 = coop.define_agent();
			a1.on_start( [&coop, a1] { so_5::send< msg_stop >(
					a1.direct_mbox() );
				} )
				.event< msg_stop >( a1.direct_mbox(), [&coop] {
					coop.environment().stop();
				} );

		} );
}

void
init( so_5::rt::environment_t & env )
{
	auto one_thread = so_5::disp::one_thread::create_private_disp( env );
	make_coop( env, one_thread );
	make_stopper( env );
}

int
main()
{
	try
	{
		for( int i = 0; i != 1000; ++i )
			run_with_time_limit(
				[]()
				{
					so_5::launch( &init );
				},
				4,
				"stopping environment via environment_t::stop()" );

		std::cout << "done" << std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

