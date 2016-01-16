/*
 * An example of using composite agent state and on_enter/on_exit handlers.
 */
#include <iostream>

#include <so_5/all.hpp>

class blinking_led final : public so_5::agent_t
{
	state_t
		off{ this, "off" },
		blinking{ this, "blinking" },
			blink_on{ initial_substate_of{ blinking }, "on" },
			blink_off{ substate_of{ blinking }, "off" };

	struct timer : public so_5::signal_t {};

public :
	struct turn : public so_5::signal_t {};

	blinking_led( context_t ctx ) : so_5::agent_t{ ctx }
	{
		this >>= off;

		off
			.just_switch_to< turn >( blinking );

		blinking
			.on_enter( [this] {
				m_timer = so_5::send_periodic< timer >(
					*this, std::chrono::seconds::zero(), std::chrono::seconds{1} );
				} )
			.on_exit( [this]{ m_timer.release(); } )
			.just_switch_to< turn >( off );

		blink_on
			.on_enter( [this]{ led_on(); } )
			.on_exit( [this]{ led_off(); } )
			.just_switch_to< timer >( blink_off );

		blink_off
			.just_switch_to< timer >( blink_on );
	}

private :
	so_5::timer_id_t m_timer;

	void led_on() { std::cout << "ON" << std::endl; }
	void led_off() { std::cout << "off" << std::endl; }
};

int main()
{
	try
	{
		so_5::launch( []( so_5::environment_t & env ) {
			so_5::mbox_t m;
			env.introduce_coop( [&]( so_5::coop_t & coop ) {
					auto led = coop.make_agent< blinking_led >();
					m = led->so_direct_mbox();
				} );

			auto pause = []( unsigned int v ) {
				std::this_thread::sleep_for( std::chrono::seconds{v} );
			};

			std::cout << "Turn blinking on for 10s" << std::endl;
			so_5::send< blinking_led::turn >( m );
			pause( 10 );

			std::cout << "Turn blinking off for 5s" << std::endl;
			so_5::send< blinking_led::turn >( m );
			pause( 5 );

			std::cout << "Turn blinking on for 5s" << std::endl;
			so_5::send< blinking_led::turn >( m );
			pause( 5 );

			std::cout << "Stopping..." << std::endl;
			env.stop();
		} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}

	return 0;
}

