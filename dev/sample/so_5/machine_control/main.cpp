/*
 * An example of using priority-respected dispatcher to imitate
 * control of some machines with engines and coolers inside.
 */

#include <iostream>

#include <so_5/all.hpp>

enum class engine_state_t { on, off };
enum class cooler_state_t { on, off };

// Signals to turn engine on and off.
struct turn_engine_on : public so_5::rt::signal_t {};
struct turn_engine_off : public so_5::rt::signal_t {};

// Signals to turn cooler on and off.
struct turn_cooler_on : public so_5::rt::signal_t {};
struct turn_cooler_off : public so_5::rt::signal_t {};

// Machine status message.
struct machine_status : public so_5::rt::message_t
{
	const std::string m_id;
	const engine_state_t m_engine_status;
	const cooler_state_t m_cooler_status;
	const float m_engine_temperature;

	machine_status(
		std::string id,
		engine_state_t engine_status,
		cooler_state_t cooler_status,
		float engine_temperature )
		:	m_id( std::move(id) )
		,	m_engine_status( engine_status )
		,	m_cooler_status( cooler_status )
		,	m_engine_temperature( engine_temperature )
	{}
};

// Agent for representing a machine.
class a_machine_t : public so_5::rt::agent_t
{
	// Periodic signal to update and distribute status of the machine.
	struct update_status : public so_5::rt::signal_t {};

public :
	a_machine_t(
		context_t ctx,
		std::string id,
		so_5::rt::mbox_t status_distribution_mbox,
		float initial_temperature,
		float engine_heating_step,
		float cooler_impact_step )
		:	so_5::rt::agent_t( ctx )
		,	m_id( std::move( id ) )
		,	m_status_distribution_mbox( std::move( status_distribution_mbox ) )
		,	m_initial_temperature( initial_temperature )
		,	m_engine_heating_step( engine_heating_step )
		,	m_cooler_impact_step( cooler_impact_step )
		,	m_engine_temperature( initial_temperature )
	{}

	virtual void so_define_agent() override
	{
		this >>= st_engine_off;

		st_engine_on
			.event< turn_engine_off >( &a_machine_t::evt_turn_engine_off )
			.event< turn_cooler_on >( &a_machine_t::evt_turn_cooler_on )
			.event< turn_cooler_off >( &a_machine_t::evt_turn_cooler_off )
			.event< update_status >( &a_machine_t::evt_update_status_when_engine_on );
		st_engine_off
			.event< turn_engine_on >( &a_machine_t::evt_turn_engine_on )
			.event< turn_cooler_on >( &a_machine_t::evt_turn_cooler_on )
			.event< turn_cooler_off >( &a_machine_t::evt_turn_cooler_off )
			.event< update_status >( &a_machine_t::evt_update_status_when_engine_off );
	}

	virtual void so_evt_start() override
	{
		// Periodic update_status signal must be initiated.
		so_5::send_periodic_to_agent< update_status >( *this,
				std::chrono::milliseconds(0),
				std::chrono::milliseconds(200) );
	}

private :
	const so_5::rt::state_t st_engine_on = so_make_state( "on" );
	const so_5::rt::state_t st_engine_off = so_make_state( "off" );

	const std::string m_id;
	const so_5::rt::mbox_t m_status_distribution_mbox;

	const float m_initial_temperature;
	const float m_engine_heating_step;
	const float m_cooler_impact_step;

	float m_engine_temperature;

	engine_state_t m_engine_status = engine_state_t::off;
	cooler_state_t m_cooler_status = cooler_state_t::off;

	void evt_turn_engine_off()
	{
		this >>= st_engine_off;
		m_engine_status = engine_state_t::off;
	}

	void evt_turn_engine_on()
	{
		this >>= st_engine_on;
		m_engine_status = engine_state_t::on;
	}

	void evt_turn_cooler_off()
	{
		m_cooler_status = cooler_state_t::off;
	}

	void evt_turn_cooler_on()
	{
		m_cooler_status = cooler_state_t::on;
	}

	void evt_update_status_when_engine_on()
	{
		m_engine_temperature += m_engine_heating_step;
		if( cooler_state_t::on == m_cooler_status )
			m_engine_temperature -= m_cooler_impact_step;

		distribute_status();
	}

	void evt_update_status_when_engine_off()
	{
		if( cooler_state_t::on == m_cooler_status )
		{
			m_engine_temperature -= m_cooler_impact_step;
			if( m_engine_temperature < m_initial_temperature )
				m_engine_temperature = m_initial_temperature;
		}

		distribute_status();
	}

	void distribute_status()
	{
		so_5::send< machine_status >(
				m_status_distribution_mbox,
				m_id,
				m_engine_status,
				m_cooler_status,
				m_engine_temperature );
	}
};

void
init( so_5::rt::environment_t & env )
{
}

int main()
{
	try
	{
		so_5::launch( &init );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

