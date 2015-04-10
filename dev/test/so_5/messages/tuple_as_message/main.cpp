/*
 * Test for so_5::rt::tuple_as_message_t.
 */

#include <iostream>
#include <map>
#include <exception>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>
		
void
create_coop(
	so_5::rt::agent_coop_t & coop )
{
	using namespace so_5;
	using namespace so_5::rt;
	using namespace std;

	using hello = tuple_as_message_t< integral_constant< int, 0 >, string >;
	using bye = tuple_as_message_t< integral_constant< int, 1 >, string, string >;
	using repeat = tuple_as_message_t< integral_constant< int, 2 >, int, int >;

	auto agent = coop.define_agent();
	agent.on_start( [agent] {
			send< hello >( agent.direct_mbox(), "Hello" );
		} );
	agent.event( agent.direct_mbox(), [agent]( const hello & evt ) {
			cout << "hello: " << get<0>( evt ) << endl;
			send< repeat >( agent.direct_mbox(), 0, 3 );
		} );
	agent.event( agent.direct_mbox(), [agent]( const repeat & evt ) {
			cout << "repetition: " << get<0>( evt ) << endl;
			auto next = get<0>( evt ) + 1;
			if( next < get<1>( evt ) )
				send< repeat >( agent.direct_mbox(), next, get<1>( evt ) );
			else
				send< bye >( agent.direct_mbox(), "Good", "Bye" );
		} );

	auto & env = coop.environment();
	agent.event( agent.direct_mbox(), [&env]( const bye & evt ) {
			cout << "bye: " << get<0>( evt ) << " " << get<1>( evt ) << endl;
			env.stop();
		} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( []( so_5::rt::environment_t & env )
					{
						env.introduce_coop( create_coop );
					} );
			},
			4,
			"introduce_coop test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

