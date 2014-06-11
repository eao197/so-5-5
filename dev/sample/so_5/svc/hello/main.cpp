/*
 * A sample for service request handler.
 */

#include <iostream>
#include <exception>
#include <sstream>

#include <so_5/rt/h/rt.hpp>
#include <so_5/api/h/api.hpp>
#include <so_5/h/types.hpp>

#include <so_5/disp/active_obj/h/pub.hpp>

class msg_hello_svc : public so_5::rt::signal_t {};

class a_hello_service_t
	:	public so_5::rt::agent_t
	{
	public :
		a_hello_service_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & self_mbox )
			:	so_5::rt::agent_t( env )
			,	m_self_mbox( self_mbox )
			{}

		virtual void
		so_define_agent()
			{
				so_subscribe( m_self_mbox )
						.event( &a_hello_service_t::svc_hello );
			}

		std::string
		svc_hello( const so_5::rt::event_data_t< msg_hello_svc > & evt )
			{
				std::cout << "svc_hello called" << std::endl;
				return "Hello, World!";
			}

	private :
		const so_5::rt::mbox_ref_t m_self_mbox;
	};

struct msg_convert : public so_5::rt::message_t
	{
		int m_value;

		msg_convert( int value ) : m_value( value )
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

		std::string
		svc_convert( const so_5::rt::event_data_t< msg_convert > & evt )
			{
				std::cout << "svc_convert called: value=" << evt->m_value
						<< std::endl;

				std::ostringstream s;
				s << evt->m_value;

				return s.str();
			}

	private :
		const so_5::rt::mbox_ref_t m_self_mbox;
	};

struct msg_shutdown : public so_5::rt::signal_t {};

class a_shutdowner_t
	:	public so_5::rt::agent_t
	{
	public :
		a_shutdowner_t(
			so_5::rt::so_environment_t & env,
			const so_5::rt::mbox_ref_t & self_mbox )
			:	so_5::rt::agent_t( env )
			,	m_self_mbox( self_mbox )
			{}

		virtual void
		so_define_agent()
			{
				so_subscribe( m_self_mbox )
						.event( &a_shutdowner_t::svc_shutdown );
			}

		void
		svc_shutdown( const so_5::rt::event_data_t< msg_shutdown > & evt )
			{
				std::cout << "svc_shutdown called" << std::endl;

				so_environment().stop();
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
				auto hello =
					//NOTE: it could be a method of agent_t.
					so_5::rt::service< std::string >( m_svc_mbox )
							.request< msg_hello_svc >();

				auto convert =
					so_5::rt::service< std::string >( m_svc_mbox )
							.request( new msg_convert( 42 ) );

				std::cout << "hello_svc: " << hello.get() << std::endl;
				std::cout << "convert_svc: " << convert.get() << std::endl;

				std::cout << "sync_convert_svc: "
						<< so_5::rt::service< std::string >( m_svc_mbox )
								.wait_forever().request( new msg_convert( 1020 ) )
						<< std::endl;

				// More complex case with conversion.
				auto svc_proxy = so_5::rt::service< std::string >( m_svc_mbox );

				// These requests should be processed before next 'sync_request'...
				auto c1 = svc_proxy.request( new msg_convert( 1 ) );
				auto c2 = svc_proxy.request( new msg_convert( 2 ) );

				// Two previous request should be processed before that call.
				std::cout << "sync_convert_svc: "
						<< svc_proxy.wait_forever().request( new msg_convert( 3 ) )
						<< std::endl;

				// But their value will be accessed only now.
				std::cout << "convert_svc: c2=" << c2.get() << std::endl;
				std::cout << "convert_svc: c1=" << c1.get() << std::endl;

				// Initiate shutdown via another synchonyous service.
				so_5::rt::service< void >( m_svc_mbox )
						.wait_forever().request< msg_shutdown >();
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

		coop->add_agent( new a_hello_service_t( env, svc_mbox ) );
		coop->add_agent( new a_convert_service_t( env, svc_mbox ) );
		coop->add_agent( new a_shutdowner_t( env, svc_mbox ) );
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

