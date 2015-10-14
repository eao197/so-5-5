/*
 * A simple test for message delivery tracing in the case of service requests
 * and an exception during delivery.
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

using counter_t = std::atomic< unsigned int >;

class tracer_t : public so_5::msg_tracing::tracer_t
{
public :
	tracer_t(
		counter_t & counter,
		so_5::msg_tracing::tracer_unique_ptr_t actual_tracer )
		:	m_counter{ counter }
		,	m_actual_tracer{ std::move( actual_tracer ) }
	{}

	virtual void
	trace( const std::string & message ) SO_5_NOEXCEPT override
	{
		++m_counter;
		m_actual_tracer->trace( message );
	}

private :
	counter_t & m_counter;
	so_5::msg_tracing::tracer_unique_ptr_t m_actual_tracer;
};

struct finish : public so_5::rt::signal_t {};

class a_request_initator_t : public so_5::rt::agent_t
{
public :
	a_request_initator_t( context_t ctx, so_5::rt::mbox_t data_mbox )
		:	so_5::rt::agent_t{ ctx }
		,	m_data_mbox{ std::move( data_mbox ) }
		{}

	virtual void
	so_evt_start() override
	{
		try
		{
			so_5::request_value< void, finish >( m_data_mbox, so_5::infinite_wait );
		}
		catch( const std::exception & x )
		{
			std::cout << "Expected exception: " << x.what() << std::endl;
			so_deregister_agent_coop_normally();
		}
	}

private :
	const so_5::rt::mbox_t m_data_mbox;
};

void
init( so_5::rt::environment_t & env )
{
	env.introduce_coop(
		so_5::disp::active_obj::create_private_disp( env )->binder(),
		[]( so_5::rt::agent_coop_t & coop ) {
			coop.make_agent< a_request_initator_t >(
					coop.environment().create_local_mbox( "gate" ) );
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
				counter_t counter = { 0 };
				so_5::launch( &init,
					[&counter]( so_5::rt::environment_params_t & params ) {
						params.message_delivery_tracer(
								so_5::msg_tracing::tracer_unique_ptr_t{
										new tracer_t{ counter,
												so_5::msg_tracing::std_cout_tracer() } } );
					} );

				const unsigned int expected_value = 2;
				auto actual_value = counter.load( std::memory_order_acquire );
				if( expected_value != actual_value )
					throw std::runtime_error( "Unexpected count of trace messages: "
							"expected=" + std::to_string(expected_value) +
							", actual=" + std::to_string(actual_value) );
			},
			4,
			"simple tracing for service request via MPMC-mboxes" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

