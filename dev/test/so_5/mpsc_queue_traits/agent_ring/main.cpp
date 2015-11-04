#include <iostream>
#include <set>
#include <chrono>

#include <cstdio>
#include <cstdlib>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

class a_ring_member_t : public so_5::rt::agent_t
	{
	public :
		struct msg_start : public so_5::rt::signal_t {};

		struct msg_your_turn
			{
				unsigned long long m_request_number;
			};

		a_ring_member_t( context_t ctx )
			:	so_5::rt::agent_t( ctx )
			{}

		void
		set_next_mbox( const so_5::rt::mbox_t & mbox )
			{
				m_next_mbox = mbox;
			}

		virtual void
		so_define_agent()
			{
				so_default_state()
					.event< msg_start >( &a_ring_member_t::evt_start )
					.event( &a_ring_member_t::evt_your_turn );
			}

		void
		evt_start()
			{
				so_5::send< msg_your_turn >( m_next_mbox, 0ull );
			}

		void
		evt_your_turn( const msg_your_turn & evt )
			{
				++m_rounds_passed;
				if( m_rounds_passed < m_rounds )
					so_5::send< msg_your_turn >( m_next_mbox, evt.m_request_number + 1 );
				else
					{
						so_environment().stop();
					}
			}

	private :
		so_5::rt::mbox_t m_self_mbox;
		so_5::rt::mbox_t m_next_mbox;

		unsigned int m_rounds_passed = 0;
		const unsigned int m_rounds = 2000;
	};

using lock_factory_t = so_5::disp::mpsc_queue_traits::lock_factory_t;

class binder_factory_t
	{
	public :
		virtual ~binder_factory_t() {}

		virtual so_5::rt::disp_binder_unique_ptr_t
		binder() = 0;
	};

using binder_factory_unique_ptr_t = std::unique_ptr< binder_factory_t >;

class one_thread_binder_factory_t : public binder_factory_t
	{
	public :
		one_thread_binder_factory_t(
			so_5::disp::one_thread::private_dispatcher_handle_t disp )
			:	m_disp{ std::move( disp ) }
			{}

		virtual so_5::rt::disp_binder_unique_ptr_t
		binder() override
			{
				return m_disp->binder();
			}
	private :
		so_5::disp::one_thread::private_dispatcher_handle_t m_disp;
	};

class active_obj_binder_factory_t : public binder_factory_t
	{
	public :
		active_obj_binder_factory_t(
			so_5::disp::active_obj::private_dispatcher_handle_t disp )
			:	m_disp{ std::move( disp ) }
			{}

		virtual so_5::rt::disp_binder_unique_ptr_t
		binder() override
			{
				return m_disp->binder();
			}
	private :
		so_5::disp::active_obj::private_dispatcher_handle_t m_disp;
	};

class active_group_binder_factory_t : public binder_factory_t
	{
	public :
		active_group_binder_factory_t(
			so_5::disp::active_group::private_dispatcher_handle_t disp )
			:	m_disp{ std::move( disp ) }
			{}

		virtual so_5::rt::disp_binder_unique_ptr_t
		binder() override
			{
				auto id = ++m_id;
				return m_disp->binder( std::to_string(id) );
			}
	private :
		so_5::disp::active_group::private_dispatcher_handle_t m_disp;

		unsigned int m_id = {0};
	};

void
create_coop(
	so_5::rt::environment_t & env,
	binder_factory_t & binder_factory )
	{
		so_5::rt::mbox_t first_agent_mbox;

		env.introduce_coop(
			[&]( so_5::rt::agent_coop_t & coop )
			{
				const std::size_t ring_size = 16;

				std::vector< a_ring_member_t * > agents;
				agents.reserve( ring_size );

				std::vector< so_5::rt::mbox_t > mboxes;
				mboxes.reserve( ring_size );

				for( unsigned int i = 0; i != ring_size; ++i )
					{
						auto member = coop.make_agent_with_binder< a_ring_member_t >(
								binder_factory.binder() );
						agents.push_back( member );
						mboxes.push_back( member->so_direct_mbox() );
					}

				for( unsigned int i = 0; i != ring_size; ++i )
					{
						agents[ i ]->set_next_mbox(
								mboxes[ (i + 1) % ring_size ] );
					}

				first_agent_mbox = mboxes[ 0 ]; 
			} );

		so_5::send< a_ring_member_t::msg_start >( first_agent_mbox );
	}

using binder_factory_maker_t = std::function<
	binder_factory_unique_ptr_t(so_5::rt::environment_t &, lock_factory_t) >;

binder_factory_unique_ptr_t
one_thread_maker(
	so_5::rt::environment_t & env,
	lock_factory_t lock_factory )
	{
		auto disp = so_5::disp::one_thread::create_private_disp(
				env,
				std::string(),
				so_5::disp::one_thread::params_t{}.tune_queue_params(
					[lock_factory]( so_5::disp::one_thread::queue_traits::params_t & p ) {
						p.lock_factory( lock_factory );
					} ) );
		binder_factory_unique_ptr_t factory{ new one_thread_binder_factory_t{
				std::move(disp) } };
		return factory;
	}

binder_factory_unique_ptr_t
active_obj_maker(
	so_5::rt::environment_t & env,
	lock_factory_t lock_factory )
	{
		auto disp = so_5::disp::active_obj::create_private_disp(
				env,
				std::string(),
				so_5::disp::active_obj::params_t{}.tune_queue_params(
					[lock_factory]( so_5::disp::active_obj::queue_traits::params_t & p ) {
						p.lock_factory( lock_factory );
					} ) );
		binder_factory_unique_ptr_t factory{ new active_obj_binder_factory_t{
				std::move(disp) } };
		return factory;
	}

binder_factory_unique_ptr_t
active_group_maker(
	so_5::rt::environment_t & env,
	lock_factory_t lock_factory )
	{
		auto disp = so_5::disp::active_group::create_private_disp(
				env,
				std::string(),
				so_5::disp::active_group::params_t{}.tune_queue_params(
					[lock_factory]( so_5::disp::active_group::queue_traits::params_t & p ) {
						p.lock_factory( lock_factory );
					} ) );
		binder_factory_unique_ptr_t factory{ new active_group_binder_factory_t{
				std::move(disp) } };
		return factory;
	}

void
do_test()
	{
		struct case_info_t
			{
				std::string m_disp_name;
				binder_factory_maker_t m_factory_maker;
			};
		std::vector< case_info_t > cases;
		cases.push_back( case_info_t{ "one_thread", one_thread_maker } );
		cases.push_back( case_info_t{ "active_obj", active_obj_maker } );
		cases.push_back( case_info_t{ "active_group", active_group_maker } );

		struct lock_factory_info_t
			{
				std::string m_name;
				lock_factory_t m_factory;
			};
		std::vector< lock_factory_info_t > factories;
		factories.push_back( lock_factory_info_t{
				"combined_lock", so_5::disp::mpsc_queue_traits::combined_lock_factory() } );
		factories.push_back( lock_factory_info_t{
				"combined_lock(1s)",
				so_5::disp::mpsc_queue_traits::combined_lock_factory(
						std::chrono::seconds(1) ) } );
		factories.push_back( lock_factory_info_t{
				"simple_lock",
				so_5::disp::mpsc_queue_traits::simple_lock_factory() } );

		for( const auto & c : cases )
			for( const auto & f : factories )
				{
					std::cout << "--- " << c.m_disp_name << "+" << f.m_name << "---"
							<< std::endl;

					run_with_time_limit( [&] {
								so_5::launch( [&]( so_5::rt::environment_t & env ) {
										auto maker = c.m_factory_maker( env, f.m_factory );
										create_coop( env, *maker );
									} );
							},
							5,
							"dispatcher: " + c.m_disp_name + ", lock: " + f.m_name );

					std::cout << "--- DONE ---" << std::endl;
				}
	}

int
main()
{
	try
	{
		do_test();

		return 0;
	}
	catch( const std::exception & x )
	{
		std::cerr << "*** Exception caught: " << x.what() << std::endl;
	}

	return 2;
}

