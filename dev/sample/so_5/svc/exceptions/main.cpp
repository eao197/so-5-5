/*
 * A sample for an exception from service handler demonstration.
 */

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <chrono>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/h/types.hpp>

#include <so_5/disp/active_obj/h/pub.hpp>

struct msg_convert : public so_5::rt::message_t
	{
		const std::string m_value;

		msg_convert( const std::string & value ) : m_value( value )
			{}
	};

class a_convert_service_t
	:	public so_5::rt::agent_t
	{
	public :
		a_convert_service_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & self_mbox )
			:	so_5::rt::agent_t( env )
			,	m_self_mbox( self_mbox )
			{}

		virtual void
		so_define_agent()
			{
				so_subscribe( m_self_mbox )
						.event( &a_convert_service_t::svc_convert );
			}

		int
		svc_convert( const so_5::rt::event_data_t< msg_convert > & evt )
			{
				std::istringstream s( evt->m_value );
				int result;
				s >> result;
				if( s.fail() )
					throw std::invalid_argument( "unable to convert to int: '" +
							evt->m_value + "'" );

				// A special case for timeout imitation.
				if( 42 == result )
					std::this_thread::sleep_for( std::chrono::milliseconds(150) );

				return result;
			}

	private :
		const so_5::rt::mbox_ref_t m_self_mbox;
	};

class a_client_t
	:	public so_5::rt::agent_t
	{
	public :
		a_client_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & svc_mbox )
			:	so_5::rt::agent_t( env )
			,	m_svc_mbox( svc_mbox )
			{}

		virtual void
		so_evt_start()
			{
				std::string values_to_convert[] = {
						"1", "2", "a1", "a2", "3", "a3", "41", "42", "43" };

				auto svc = m_svc_mbox->get_one< int >().wait_for(
						std::chrono::milliseconds( 100 ) );

				for( const auto & s : values_to_convert )
					{
						try
							{
								std::cout << "converting '" << s << "'" << std::flush;

								auto r = svc.sync_get( new msg_convert( s ) );

								std::cout << "' -> " << r << std::endl;
							}
						catch( const std::exception & x )
							{
								std::cerr << "\n*** an exception during converting "
										"value '" << s << "': " << x.what()
										<< std::endl;
							}
					}

				so_environment().stop();
			}

	private :
		const so_5::rt::mbox_ref_t m_svc_mbox;
	};

void
init(
	so_5::rt::so_environment_t & env )
	{
		auto coop = env.create_coop(
				so_5::rt::nonempty_name_t( "test_coop" ),
				so_5::disp::active_obj::create_disp_binder( "active_obj" ) );

		auto svc_mbox = env.create_local_mbox();

		coop->add_agent( new a_convert_service_t( env, svc_mbox ) );
		coop->add_agent( new a_client_t( env, svc_mbox ) );

		env.register_coop( std::move( coop ) );
	}

int
main( int, char ** )
	{
		try
			{
				so_5::api::run_so_environment(
					&init,
					std::move(
						so_5::rt::so_environment_params_t()
							.add_named_dispatcher(
								so_5::rt::nonempty_name_t( "active_obj" ),
								so_5::disp::active_obj::create_disp() ) ) );
			}
		catch( const std::exception & ex )
			{
				std::cerr << "Error: " << ex.what() << std::endl;
				return 1;
			}

		return 0;
	}

