/*
 * A test for checking message subscription/unsubscription inside
 * agent's states.
*/

#include <iostream>
#include <exception>
#include <stdexcept>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

struct test_message
	:
		public so_5::rt::message_t
{
	test_message() {}
	virtual ~test_message(){}

};

class test_agent_t
	:
		public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;
		const so_5::rt::state_t m_first_state;
		const so_5::rt::state_t m_second_state;
		const so_5::rt::state_t m_third_state;

	public:
		test_agent_t(
			so_5::rt::so_environment_t & env )
			:
				base_type_t( env ),
				m_first_state( self_ptr() ),
				m_second_state( self_ptr() ),
				m_third_state( self_ptr() ),
				m_test_mbox( so_environment().create_local_mbox() )
		{}

		virtual ~test_agent_t()
		{}

		virtual void
		so_define_agent();

		virtual void
		so_evt_start();

		void
		evt_in_state_default(
			const so_5::rt::event_data_t< test_message > & );

		void
		evt_in_state_1(
			const so_5::rt::event_data_t< test_message > & );

		void
		evt_in_state_2(
			const so_5::rt::event_data_t< test_message > & );

		void
		evt_in_state_3(
			const so_5::rt::event_data_t< test_message > & );

		// How much every handler has been called.
		static int m_handler_in_state_default_calls;
		static int m_handler_in_state_1_calls;
		static int m_handler_in_state_2_calls;
		static int m_handler_in_state_3_calls;

	private:
		so_5::rt::mbox_ref_t m_test_mbox;
};

int test_agent_t::m_handler_in_state_default_calls = 0;
int test_agent_t::m_handler_in_state_1_calls = 0;
int test_agent_t::m_handler_in_state_2_calls = 0;
int test_agent_t::m_handler_in_state_3_calls = 0;

template< class T, class V >
struct method_ptr_dumper_t
{
	typedef void (T::*method_ptr)(V);

	method_ptr m_ptr;
	method_ptr_dumper_t( method_ptr ptr ) : m_ptr(ptr) {}
};

template< class T, class V >
method_ptr_dumper_t< T, V >
method_ptr_dumper( void (T::*ptr)(V) )
{
	return method_ptr_dumper_t<T, V>(ptr);
}

template< class T, class V >
std::ostream &
operator<<( std::ostream & to, method_ptr_dumper_t<T, V> a )
{
	const char * p = (const char *)&a.m_ptr;
	for( size_t i = 0; i != sizeof(a.m_ptr); ++i )
	{
		to << static_cast<short>(p[i]) << ",";
	}
	return to;
}

void
test_agent_t::so_define_agent()
{
	// Subscribe for message in default state...
	so_subscribe( m_test_mbox )
		.in( so_default_state() )
			.event(
				&test_agent_t::evt_in_state_default,
				so_5::THROW_ON_ERROR );

	// ...in first state...
	so_subscribe( m_test_mbox )
		.in( m_first_state )
			.event(
				&test_agent_t::evt_in_state_1,
				so_5::THROW_ON_ERROR );

	// ...in second state...
	so_subscribe( m_test_mbox )
		.in( m_second_state )
			.event(
				&test_agent_t::evt_in_state_2,
				so_5::THROW_ON_ERROR );

	// ...in third state...
	so_subscribe( m_test_mbox )
		.in( m_third_state )
			.event(
				&test_agent_t::evt_in_state_3,
				so_5::THROW_ON_ERROR );
}

void
test_agent_t::so_evt_start()
{
	// Unsubscribe from message in default state...
	so_unsubscribe( m_test_mbox )
		.in( so_default_state() )
			.event(
				&test_agent_t::evt_in_state_default,
				so_5::THROW_ON_ERROR );

	// ...in first state...
	so_unsubscribe( m_test_mbox )
		.in( m_first_state )
			.event(
				&test_agent_t::evt_in_state_1,
				so_5::THROW_ON_ERROR );

	// ...keep subscription for second state...

	// ...but unsubscribe in third state...
	so_unsubscribe( m_test_mbox )
		.in( m_third_state )
			.event(
				&test_agent_t::evt_in_state_3,
				so_5::THROW_ON_ERROR );

	m_test_mbox->deliver_message< test_message >();

	// Change state.
	so_change_state( m_second_state, so_5::THROW_ON_ERROR );
}

void
test_agent_t::evt_in_state_default(
	const so_5::rt::event_data_t< test_message > & )
{
	// Expect this handler will not be called.
	++m_handler_in_state_default_calls;
}

void
test_agent_t::evt_in_state_1(
	const so_5::rt::event_data_t< test_message > & )
{
	// Expect this handler will not be called.
	++m_handler_in_state_1_calls;
}

void
test_agent_t::evt_in_state_2(
	const so_5::rt::event_data_t< test_message > & )
{
	++m_handler_in_state_2_calls;
	if( 4 == m_handler_in_state_2_calls )
		so_environment().stop();
}

void
test_agent_t::evt_in_state_3(
	const so_5::rt::event_data_t< test_message > & )
{
	// Expect this handler will not be called.
	++m_handler_in_state_3_calls;
}

void
init( so_5::rt::so_environment_t & env )
{
	so_5::rt::agent_coop_unique_ptr_t coop =
		env.create_coop( "test_coop" );

	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );
	coop->add_agent( so_5::rt::agent_ref_t(
		new test_agent_t( env ) ) );

	env.register_coop(
		std::move( coop ),
		so_5::THROW_ON_ERROR );
}

int
main( int argc, char * argv[] )
{
	try
	{
		so_5::api::run_so_environment(
			&init,
			so_5::rt::so_environment_params_t()
				.mbox_mutex_pool_size( 4 )
				.agent_event_queue_mutex_pool_size( 4 ) );

		if( test_agent_t::m_handler_in_state_default_calls != 0 ||
			test_agent_t::m_handler_in_state_1_calls != 0 ||
			test_agent_t::m_handler_in_state_2_calls != 4 ||
			test_agent_t::m_handler_in_state_3_calls != 0 )
		{
			std::cerr
				<< "test_agent_t::m_handler_in_state_default_calls = "
				<< test_agent_t::m_handler_in_state_default_calls << "\n"
				<< "test_agent_t::m_handler_in_state_1_calls = "
				<< test_agent_t::m_handler_in_state_1_calls << "\n"
				<< "test_agent_t::m_handler_in_state_2_calls = "
				<< test_agent_t::m_handler_in_state_2_calls << "\n"
				<< "test_agent_t::m_handler_in_state_3_calls = "
				<< test_agent_t::m_handler_in_state_3_calls << "\n";

			throw std::runtime_error( "handler calls count error" );
		}

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

