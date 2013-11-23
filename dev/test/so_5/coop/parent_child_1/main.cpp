/*
 * A test for sequence of child cooperation deregistration.
 */

#include <iostream>
#include <sstream>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

struct msg_child_started : public so_5::rt::signal_t {};

void
create_and_register_agent(
	so_5::rt::so_environment_t & env,
	int ordinal,
	int max_deep );

class a_test_t : public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public :
		a_test_t(
			so_5::rt::so_environment_t & env,
			int ordinal,
			int max_deep )
			:	base_type_t( env )
			,	m_ordinal( ordinal )
			,	m_max_deep( max_deep )
			,	m_self_mbox(
					env.create_local_mbox( mbox_name( ordinal ) ) )
		{
		}

		~a_test_t()
		{
		}

		void
		so_define_agent()
		{
			so_subscribe( m_self_mbox )
				.event( &a_test_t::evt_child_started );
		}

		void
		so_evt_start()
		{
			if( m_ordinal != m_max_deep )
				create_and_register_agent(
						so_environment(),
						m_ordinal + 1,
						m_max_deep );
			else
				notify_parent();
		}

		void
		evt_child_started(
			const so_5::rt::event_data_t< msg_child_started > & )
		{
			if( m_ordinal )
				notify_parent();
			else
				so_environment().deregister_coop( so_coop_name() );
		}

	private :
		const int m_ordinal;

		const int m_max_deep;

		so_5::rt::mbox_ref_t m_self_mbox;

		static std::string
		mbox_name( int ordinal )
		{
			std::ostringstream s;
			s << "agent_" << ordinal;
			return s.str();
		}

		void
		notify_parent()
		{
			so_environment().create_local_mbox( mbox_name( m_ordinal - 1 ) )->
					deliver_signal< msg_child_started >();
		}
};

std::string
create_coop_name( int ordinal )
{
	std::ostringstream s;
	s << "coop_" << ordinal;
	return s.str();
}

void
create_and_register_agent(
	so_5::rt::so_environment_t & env,
	int ordinal,
	int max_deep )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			create_coop_name( ordinal ) );
	if( ordinal )
		coop->set_parent_coop_name( create_coop_name( ordinal - 1 ) );

	coop->add_agent( new a_test_t( env, ordinal, max_deep ) );

	env.register_coop( std::move( coop ) );
}

struct init_deinit_data_t
{
	std::vector< std::string > m_init_sequence;
	std::vector< std::string > m_deinit_sequence;
};

class test_coop_listener_t
	:	public so_5::rt::coop_listener_t
{
	public :
		test_coop_listener_t( init_deinit_data_t & data )
			:	m_data( data )
			,	m_active_coops( 0 )
		{}

		virtual void
		on_registered(
			so_5::rt::so_environment_t & env,
			const std::string & coop_name )
		{
			std::cout << "registered: " << coop_name << std::endl;

			m_data.m_init_sequence.push_back( coop_name );

			++m_active_coops;
		}

		virtual void
		on_deregistered(
			so_5::rt::so_environment_t & env,
			const std::string & coop_name )
		{
			std::cout << "deregistered: " << coop_name << std::endl;

			m_data.m_deinit_sequence.insert(
					m_data.m_deinit_sequence.begin(),
					coop_name );

			--m_active_coops;

			if( !m_active_coops )
				env.stop();
		}

		static so_5::rt::coop_listener_unique_ptr_t
		make( init_deinit_data_t & data )
		{
			return so_5::rt::coop_listener_unique_ptr_t(
					new test_coop_listener_t( data ) );
		}

	private :
		init_deinit_data_t & m_data;

		int m_active_coops;
};

class test_env_t
{
	public :
		void
		init( so_5::rt::so_environment_t & env )
		{
			create_and_register_agent( env, 0, 5 );
		}

		so_5::rt::coop_listener_unique_ptr_t
		make_listener()
		{
			return test_coop_listener_t::make( m_data );
		}

		void
		check_result() const
		{
			if( m_data.m_init_sequence != m_data.m_deinit_sequence )
				throw std::runtime_error( "Wrong deinit sequence" );
		}

	private :
		init_deinit_data_t m_data;
};

int
main( int argc, char * argv[] )
{
	try
	{
		test_env_t test_env;
		so_5::api::run_so_environment_on_object(
				test_env,
				&test_env_t::init,
				std::move( so_5::rt::so_environment_params_t().
						coop_listener( test_env.make_listener() ) ) );

		test_env.check_result();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
