/*
 * An example of implementation of hierarchical state machine by using
 * agent's states.
 */

#include <iostream>
#include <string>
#include <cctype>

// Main SObjectizer header file.
#include <so_5/all.hpp>

// Messages to be used for interaction with intercom agents.
struct key_cancel : public so_5::signal_t {};
struct key_bell : public so_5::signal_t {};
struct key_grid : public so_5::signal_t {};

struct key_digit
{
	char m_value;
};

class intercom_controller final : public so_5::agent_t
{
	// Intercom is inactive, light and display are turned off.
	state_t inactive{ this, "inactive" };

	// Intercom is active, light and display are turned on.
	state_t active{ this, "active" };

	// An initial substate of active state.
	state_t wait_activity{ initial_substate_of{ active }, "wait_activity" };

	// State for accumulating an appartment number.
	state_t number_selection{ substate_of{ active }, "number_selection" };

	// State for dialling to an appartment.
	state_t dialling{ substate_of{ active }, "dialling" };

	// State for accumulating an appartment secret code or service code.
	state_t special_code_selection{ substate_of{ active }, "special_code_selection" };

	// State for accumulation of an appartment secret code.
	state_t user_code_selection{
			initial_substate_of{ special_code_selection },
			"user_code_selection" };

	// The first part of accumulation of an appartment secret code.
	state_t user_code_appartment_number{
			initial_substate_of{ user_code_selection },
			"appartment_number" };

	// The second part of accumulation of an appartment secret code.
	state_t user_code_secret{
			substate_of{ user_code_selection },
			"secret_code" };

	// State for accumulation of servide code.
	state_t service_code_selection{
			substate_of{ special_code_selection },
			"service_code" };

	// State for door opening after secret or service code selection.
	state_t door_opening{
			substate_of{ special_code_selection },
			"door_opening" };

public :
	intercom_controller(
		context_t ctx,
		so_5::mbox_t intercom_mbox )
		:	so_5::agent_t{ ctx }
		,	m_intercom_mbox{ std::move(intercom_mbox) }
	{
		inactive
			.on_enter( [this] { on_enter_inactive(); } )
			.transfer_to_state< key_digit >( m_intercom_mbox, active )
			.transfer_to_state< key_grid >( m_intercom_mbox, active )
			.transfer_to_state< key_bell >( m_intercom_mbox, active )
			.transfer_to_state< key_cancel >( m_intercom_mbox, active );

		active
			.on_enter( [this] { on_enter_active(); } )
			.transfer_to_state< key_digit >( m_intercom_mbox, number_selection )
			.event< key_grid >( m_intercom_mbox, &intercom_controller::evt_first_grid )
			.event< key_cancel >( m_intercom_mbox, &intercom_controller::evt_cancel );

		wait_activity
			.transfer_to_state< key_digit >( m_intercom_mbox, number_selection )
			.event< key_grid >( m_intercom_mbox, &intercom_controller::evt_first_grid );

		number_selection
			.event( m_intercom_mbox, &intercom_controller::evt_appartment_number_digit );

		special_code_selection
			.transfer_to_state< key_digit >( m_intercom_mbox, user_code_selection )
			.event< key_grid >( m_intercom_mbox, [this] { this >>= service_code_selection; } );
	}

	virtual void so_evt_start() override
	{
		this >>= inactive;
	}

private :
	const so_5::mbox_t m_intercom_mbox;

	void on_enter_inactive()
	{
		// Turn light off, turn display off.
	}

	void on_enter_active()
	{
std::cout << "enter to active" << std::endl;
		// Turn light on, turn display on.
	}

	void evt_cancel()
	{
		this >>= wait_activity;
	}

	void evt_first_grid()
	{
		this >>= special_code_selection;
	}

	void evt_appartment_number_digit( const key_digit & msg )
	{
//FIXME: implement this!
	}

};

so_5::mbox_t create_intercom( so_5::environment_t & env )
{
	so_5::mbox_t intercom_mbox;
	env.introduce_coop( [&]( so_5::coop_t & coop ) {
		intercom_mbox = env.create_mbox();

		coop.make_agent< intercom_controller >( intercom_mbox );
	} );

	return intercom_mbox;
}

void demo()
{
	// A SObjectizer instance.
	so_5::wrapped_env_t sobj;

	auto intercom = create_intercom( sobj.environment() );

	while( true )
	{
		std::cout << "enter digit or 'c' or 'b' or '#' (or 'exit' to stop): "
			<< std::flush;

		std::string choice;
		std::cin >> choice;

		if( "c" == choice ) so_5::send< key_cancel >( intercom );
		else if( "b" == choice ) so_5::send< key_bell >( intercom );
		else if( "#" == choice ) so_5::send< key_grid >( intercom );
		else if( "exit" == choice ) break;
		else if( 1 == choice.size() && std::isdigit( choice[ 0 ] ) )
			so_5::send< key_digit >( intercom, choice[ 0 ] );
	}

	// SObjectizer will be stopped automatically.
}

int main()
{
	try
	{
		demo();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}

	return 0;
}

