/*
 * A test for mpsc_mbox.
 */

#include <iostream>
#include <sstream>

#include <ace/OS.h>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>

#include <so_5/disp/active_obj/h/pub.hpp>

struct msg_one : public so_5::rt::signal_t {};
struct msg_two : public so_5::rt::signal_t {};
struct msg_three : public so_5::rt::signal_t {};
struct msg_four : public so_5::rt::signal_t {};

class a_test_t : public so_5::rt::agent_t
{
		typedef so_5::rt::agent_t base_type_t;

	public :
		a_test_t(
			so_5::rt::so_environment_t & env,
			std::string & sequence )
			:	base_type_t( env )
			,	m_sequence( sequence )
			,	m_mbox( env.create_mpsc_mbox( self_ptr() ) )
		{
		}

		const so_5::rt::mbox_ref_t &
		mbox() const
		{
			return m_mbox;
		}

		void
		so_define_agent()
		{
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_one );
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_three );
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_four );
		}

		void
		evt_one( const so_5::rt::event_data_t< msg_one > & )
		{
			m_sequence += "e1:";
		}

		void
		evt_two( const so_5::rt::event_data_t< msg_two > & )
		{
			m_sequence += "e2:";
		}

		void
		evt_three( const so_5::rt::event_data_t< msg_three > & )
		{
			m_sequence += "e3:";

			so_drop_subscription( m_mbox, &a_test_t::evt_one );

			so_subscribe( m_mbox ).event( &a_test_t::evt_two );
		}

		void
		evt_four( const so_5::rt::event_data_t< msg_four > & )
		{
			m_sequence += "e4:";

			so_environment().stop();
		}

	private :
		std::string & m_sequence;

		const so_5::rt::mbox_ref_t m_mbox;
};

int
main( int argc, char * argv[] )
{
	try
	{
		std::string sequence;

		so_5::api::run_so_environment(
			[&sequence]( so_5::rt::so_environment_t & env )
			{
				auto coop = env.create_coop( "test",
					so_5::disp::active_obj::create_disp_binder( "active_obj" ) );

				auto a_test = new a_test_t( env, sequence );
				const so_5::rt::mbox_ref_t mbox = a_test->mbox();

				coop->add_agent( a_test );

				coop->define_agent()
					.on_start( [mbox]() 
						{
							mbox->deliver_signal< msg_one >();
							mbox->deliver_signal< msg_two >();
							mbox->run_one()
								.wait_for( std::chrono::seconds(1) )
								.sync_get< msg_three >();

							mbox->deliver_signal< msg_one >();
							mbox->deliver_signal< msg_two >();

							mbox->deliver_signal< msg_four >();
						} );

				env.register_coop( std::move( coop ) );
			},
			[]( so_5::rt::so_environment_params_t & params )
			{
				params.add_named_dispatcher( "active_obj",
					so_5::disp::active_obj::create_disp() );
			} );

		const std::string expected = "e1:e3:e2:e4:";
		if( sequence != expected )
			throw std::runtime_error( "sequence mismatch! "
					"expected: '" + expected + "', actual: '"
					+ sequence + "'" );

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

