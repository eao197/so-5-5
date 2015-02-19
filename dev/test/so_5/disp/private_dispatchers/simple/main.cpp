/*
 * A simple test for private dispatchers.
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

struct msg_start : public so_5::rt::signal_t {};

struct msg_hello : public so_5::rt::message_t
{
	std::string m_who;

	msg_hello( std::string who ) : m_who( std::move( who ) )
	{}
};

class a_collector_t : public so_5::rt::agent_t
{
private :
	const so_5::rt::mbox_t m_start_mbox;
	unsigned int m_remaining;	

public :
	a_collector_t(
		so_5::rt::environment_t & env,
		const so_5::rt::mbox_t & start_mbox,
		unsigned int messages_to_receive )
		:	so_5::rt::agent_t( env )
		,	m_start_mbox( start_mbox )
		,	m_remaining( messages_to_receive )
	{}

	virtual void
	so_define_agent() override
	{
		so_default_state().event( [this]( const msg_hello & msg ) {
				std::cout << "received: " << msg.m_who << std::endl;
				if( 0 == (--m_remaining) )
					so_deregister_agent_coop_normally();
			} );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send< msg_start >( m_start_mbox );
	}
};

std::string
make_hello_string( const char * who )
{
	std::ostringstream ss;
	ss << who << " from thread [" << so_5::query_current_thread_id() << "]";
	return ss.str();
}

void
init( so_5::rt::environment_t & env )
{
	auto one_thread = so_5::disp::one_thread::create_private_disp();

	auto start_mbox = env.create_local_mbox( "start" );
	auto coop = env.create_coop( so_5::autoname );
	
	auto collector =
			coop->add_agent( new a_collector_t( env, start_mbox, 1 ) );

	coop->define_agent( one_thread->binder() )
		.event< msg_start >( start_mbox,
				[collector]() {
					so_5::send_to_agent< msg_hello >( *collector,
							make_hello_string( "one_thread" ) );
				} );

	env.register_coop( std::move( coop ) );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( &init );
			},
			5,
			"simple private dispatchers test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

