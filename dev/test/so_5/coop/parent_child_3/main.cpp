/*
 * A test for sequence of child cooperation deregistration.
 */

#include <iostream>
#include <sstream>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

struct init_deinit_data_t
{
	std::vector< int > m_init_sequence;
	std::vector< int > m_deinit_sequence;
};

struct msg_child_started : public so_5::rt::signal_t {};

void
create_and_register_agent(
	so_5::rt::so_environment_t & env,
	init_deinit_data_t & data,
	int ordinal,
	int max_deep );

class a_test_t : public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public :
		a_test_t(
			so_5::rt::so_environment_t & env,
			init_deinit_data_t & data,
			int ordinal,
			int max_deep )
			:	base_type_t( env )
			,	m_data( data )
			,	m_ordinal( ordinal )
			,	m_max_deep( max_deep )
			,	m_self_mbox(
					env.create_local_mbox( mbox_name( ordinal ) ) )
		{
			m_data.m_init_sequence.push_back( m_ordinal );
		}

		~a_test_t()
		{
			m_data.m_deinit_sequence.push_back( m_ordinal );
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
						m_data,
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
				so_environment().stop();
		}

	private :
		init_deinit_data_t & m_data;

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
	init_deinit_data_t & data,
	int ordinal,
	int max_deep )
{
	so_5::rt::agent_coop_unique_ptr_t coop = env.create_coop(
			create_coop_name( ordinal ) );
	if( ordinal )
		coop->set_parent_coop_name( create_coop_name( ordinal - 1 ) );

	coop->add_agent( new a_test_t( env, data, ordinal, max_deep ) );

	env.register_coop( std::move( coop ) );
}

class test_env_t
{
	public :
		void
		init( so_5::rt::so_environment_t & env )
		{
			create_and_register_agent( env, m_data, 0, 5 );
		}

		void
		check_result() const
		{
			std::vector< int > r = m_data.m_deinit_sequence;
			std::reverse( r.begin(), r.end() );

			if( m_data.m_init_sequence != r )
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
				so_5::rt::so_environment_params_t() );

		test_env.check_result();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
