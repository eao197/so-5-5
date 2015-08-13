/*
 * An example of using priority-respected dispatcher to imitate
 * control of some machines with engines and coolers inside.
 */

#include <iostream>
#include <map>

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

// Type of reaction to be performed on machine.
enum class attention_type_t 
{
	none,
	engine_cooling_done,
	engine_cooling_needed,
	engine_overheat_detected
};

// Notification about machine which needs some attention.
struct machine_needs_attention : public so_5::rt::message_t
{
	const std::string m_id;
	const attention_type_t m_attention;
	const engine_state_t m_engine_status;
	const cooler_state_t m_cooler_status;

	machine_needs_attention(
		std::string id,
		attention_type_t attention,
		engine_state_t engine_status,
		cooler_state_t cooler_status )
		:	m_id( std::move(id) )
		,	m_attention( attention )
		,	m_engine_status( engine_status )
		,	m_cooler_status( cooler_status )
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
		m_update_status_timer = so_5::send_periodic_to_agent< update_status >(
				*this,
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

	// Timer ID for periodic update_status.
	so_5::timer_id_t m_update_status_timer;

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

// An agent to collect and periodically show status of all machines.
class a_total_status_dashboard_t : public so_5::rt::agent_t
{
	// A signal to show the current state of all machines to the console.
	struct show_dashboard : public so_5::rt::signal_t {};

public :
	a_total_status_dashboard_t(
		context_t ctx,
		so_5::rt::mbox_t status_distribution_mbox )
		:	so_5::rt::agent_t( ctx )
		,	m_status_distribution_mbox( std::move( status_distribution_mbox ) )
	{}

	virtual void so_define_agent() override
	{
		so_subscribe( m_status_distribution_mbox )
			.event( &a_total_status_dashboard_t::evt_machine_status );

		so_subscribe_self().event< show_dashboard >(
				&a_total_status_dashboard_t::evt_show_dashboard );
	}

	virtual void so_evt_start() override
	{
		// Periodic signal must be initiated.
		const auto period = std::chrono::milliseconds( 1500 );
		m_show_timer = so_5::send_periodic_to_agent< show_dashboard >( *this,
				period, period );
	}

private :
	const so_5::rt::mbox_t m_status_distribution_mbox;

	// Description of one machine state.
	struct one_machine_status_t
	{
		engine_state_t m_engine_status;
		cooler_state_t m_cooler_status;
		float m_engine_temperature;
	};

	// Type of map from machine ID to machine state.
	using machine_status_map_t = std::map< std::string, one_machine_status_t >;

	// Current statues of machines.
	machine_status_map_t m_machine_statuses;

	// Timer ID for show_dashboard periodic message.
	so_5::timer_id_t m_show_timer;

	void evt_machine_status( const machine_status & status )
	{
		m_machine_statuses[ status.m_id ] = one_machine_status_t{
				status.m_engine_status, status.m_cooler_status,
				status.m_engine_temperature
			};
	}

	void evt_show_dashboard()
	{
		auto old_precision = std::cout.precision( 5 );
		std::cout << "=== The current status ===" << std::endl;

		for( const auto m : m_machine_statuses )
		{
			show_one_status( m );
		}

		std::cout << "==========================" << std::endl;
		std::cout.precision( old_precision );
	}

	void show_one_status( const machine_status_map_t::value_type & v )
	{
		std::cout << v.first << ": e["
				<< (engine_state_t::on == v.second.m_engine_status ?
						"ON " : "off") << "] c["
				<< (cooler_state_t::on == v.second.m_cooler_status ?
						"ON " : "off") << "] t="
				<< v.second.m_engine_temperature
				<< std::endl;
	}
};

// A dictionary to get machine's mbox from machine name.
class machine_dictionary_t
{
public :
	using dictionary_type_t = std::map< std::string, so_5::rt::mbox_t >;

	machine_dictionary_t( dictionary_type_t values )
		:	m_dictionary( std::move( values ) )
	{}

	so_5::rt::mbox_t find_mbox( const std::string & name ) const
	{
		auto r = m_dictionary.find( name );
		if( r == m_dictionary.end() )
			throw std::runtime_error( "machine not found: " + name );

		return r->second;
	}

	template< typename L >
	void for_each( L lambda ) const
	{
		for( const auto m : m_dictionary )
			lambda( m.first, m.second );
	}

private :
	const dictionary_type_t m_dictionary;
};

// Agent which does analyzing of machine statuses and produces
// machine_needs_attention messages.
class a_statuses_analyzer_t : public so_5::rt::agent_t
{
public :
	a_statuses_analyzer_t(
		context_t ctx,
		so_5::rt::mbox_t status_distribution_mbox,
		float safe_temperature,
		float warn_temperature,
		float high_temperature)
		:	so_5::rt::agent_t( ctx )
		,	m_status_distribution_mbox( std::move( status_distribution_mbox ) )
		,	m_safe_temperature( safe_temperature )
		,	m_warn_temperature( warn_temperature )
		,	m_high_temperature( high_temperature )
	{}

	virtual void so_define_agent() override
	{
		so_subscribe( m_status_distribution_mbox ).event(
				&a_statuses_analyzer_t::evt_machine_status );
	}

private :
	const so_5::rt::mbox_t m_status_distribution_mbox;

	const float m_safe_temperature;
	const float m_warn_temperature;
	const float m_high_temperature;

	// Info about last known machine status.
	struct last_machine_info_t
	{
		attention_type_t m_attention;
		float m_engine_temperature;
	};

	// Map from machine ID to last known status.
	using last_info_map_t = std::map< std::string, last_machine_info_t >;

	last_info_map_t m_last_infos;

	void evt_machine_status( const machine_status & status )
	{
		auto it = m_last_infos.find( status.m_id );
		if( it == m_last_infos.end() )
			// There is no information about this machine yet.
			// It must be added.
			it = m_last_infos.insert( last_info_map_t::value_type {
					status.m_id,
					last_machine_info_t {
						attention_type_t::none,
						status.m_engine_temperature
					} } ).first;

		handle_new_status( status, it->second );
	}

	void handle_new_status(
		const machine_status & status,
		last_machine_info_t & last_info ) const
	{
		const auto fresh_info = last_machine_info_t {
				detect_attention_needed( status, last_info ),
				status.m_engine_temperature
			};

		if( last_info.m_attention != fresh_info.m_attention )
			// Machine needs some new attention.
			so_5::send< machine_needs_attention >(
					m_status_distribution_mbox,
					status.m_id,
					fresh_info.m_attention,
					status.m_engine_status,
					status.m_cooler_status );

		last_info = fresh_info;
	}

	attention_type_t detect_attention_needed(
		const machine_status & status,
		const last_machine_info_t & last ) const
	{
		if( last.m_engine_temperature < status.m_engine_temperature )
		{
			// Engine is warming.
			if( status.m_engine_temperature > m_high_temperature )
			{
				if( attention_type_t::engine_overheat_detected != last.m_attention )
					return attention_type_t::engine_overheat_detected;
			}
			else if( status.m_engine_temperature > m_warn_temperature )
			{
				if( attention_type_t::engine_cooling_needed != last.m_attention )
					return attention_type_t::engine_cooling_needed;
			}
		}
		else
		{
			// Engine is cooling.
			if( status.m_engine_temperature < m_safe_temperature )
				if( attention_type_t::none != last.m_attention &&
						attention_type_t::engine_cooling_done != last.m_attention )
					return attention_type_t::engine_cooling_done;
		}

		// Attention need not to be changed.
		return last.m_attention;
	}
};

// A base class for machine controllers.
class a_machine_controller_t : public so_5::rt::agent_t
{
public :
	using filter_t = std::function< bool(const machine_needs_attention &) >;
	using actor_t = std::function<
		void(const machine_dictionary_t &, const machine_needs_attention &) >;

	a_machine_controller_t(
		context_t ctx,
		so_5::priority_t priority,
		so_5::rt::mbox_t status_distribution_mbox,
		const machine_dictionary_t & machines,
		filter_t filter,
		actor_t actor )
		:	so_5::rt::agent_t( ctx + priority )
		,	m_status_distribution_mbox( std::move( status_distribution_mbox ) )
		,	m_machines( machines )
		,	m_filter( std::move( filter ) )
		,	m_actor( std::move( actor ) )
	{}

	virtual void so_define_agent() override
	{
		so_set_delivery_filter( m_status_distribution_mbox,
			[this]( const machine_needs_attention & msg ) { return m_filter( msg ); } );

		so_subscribe( m_status_distribution_mbox )
			.event( [this]( const machine_needs_attention & evt ) {
					m_actor( m_machines, evt );
				} );
	}

private :
	const so_5::rt::mbox_t m_status_distribution_mbox;

	const machine_dictionary_t & m_machines;

	const filter_t m_filter;
	const actor_t m_actor;
};

//
// Implementation of engine stopper.
//
bool engine_stopper_filter( const machine_needs_attention & msg )
{
	return msg.m_attention == attention_type_t::engine_overheat_detected;
}

void engine_stopper_action(
	const machine_dictionary_t & machines,
	const machine_needs_attention & evt )
{
	so_5::send< turn_engine_off >( machines.find_mbox( evt.m_id ) );
}

//
// Implementation of engine starter.
//
bool engine_starter_filter( const machine_needs_attention & msg )
{
	return msg.m_attention == attention_type_t::engine_cooling_done &&
			msg.m_engine_status == engine_state_t::off;
}

void engine_starter_action(
	const machine_dictionary_t & machines,
	const machine_needs_attention & evt )
{
	so_5::send< turn_engine_on >( machines.find_mbox( evt.m_id ) );
}

//
// Implementation of cooler starter.
//
bool cooler_starter_filter( const machine_needs_attention & msg )
{
	return (msg.m_attention == attention_type_t::engine_overheat_detected ||
			msg.m_attention == attention_type_t::engine_cooling_needed) &&
			msg.m_cooler_status == cooler_state_t::off;
}

void cooler_starter_action(
	const machine_dictionary_t & machines,
	const machine_needs_attention & evt )
{
	so_5::send< turn_cooler_on >( machines.find_mbox( evt.m_id ) );
}

//
// Implementation of cooler stopper.
//
bool cooler_stopper_filter( const machine_needs_attention & msg )
{
	return msg.m_attention == attention_type_t::engine_cooling_done &&
			msg.m_cooler_status == cooler_state_t::on;
}

void cooler_stopper_action(
	const machine_dictionary_t & machines,
	const machine_needs_attention & evt )
{
	so_5::send< turn_cooler_off >( machines.find_mbox( evt.m_id ) );
}

// Helper for creation of machine agent and adding its info into
// machine dictionary.
template< typename... ARGS >
void make_machine(
	so_5::rt::agent_coop_t & coop,
	machine_dictionary_t::dictionary_type_t & dict,
	const so_5::disp::one_thread::private_dispatcher_handle_t & disp,
	const std::string & name,
	ARGS &&... args )
{
	auto machine = coop.make_agent_with_binder< a_machine_t >(
			disp->binder(),
			name, std::forward<ARGS>(args)... );
	dict[ name ] = machine->so_direct_mbox();
}

// Helper for creation of machine agents.
const machine_dictionary_t & create_machines(
	so_5::rt::agent_coop_t & coop,
	const so_5::rt::mbox_t & status_distrib_mbox )
{
	// Data for machine dictionary.
	machine_dictionary_t::dictionary_type_t dict_data;

	// All machines will work on dedicated working thread.
	auto machine_disp = so_5::disp::one_thread::create_private_disp(
			coop.environment() );

	make_machine( coop, dict_data, machine_disp,
			"Mch01", status_distrib_mbox, 20.0f, 0.3f, 0.2f );
	make_machine( coop, dict_data, machine_disp,
			"Mch02", status_distrib_mbox, 20.0f, 0.45f, 0.2f );
	make_machine( coop, dict_data, machine_disp,
			"Mch03", status_distrib_mbox, 20.0f, 0.25f, 0.3f );
	make_machine( coop, dict_data, machine_disp,
			"Mch04", status_distrib_mbox, 20.0f, 0.26f, 0.27f );

	// Machine dictionary could be created at that point.
	return *( coop.take_under_control(
			new machine_dictionary_t( std::move( dict_data ) ) ) );
}

void create_starter_agent(
	so_5::rt::agent_coop_t & coop,
	const machine_dictionary_t & dict )
{
	coop.define_agent().on_start( [&dict] {
			dict.for_each(
				[]( const std::string &, const so_5::rt::mbox_t & mbox ) {
					so_5::send< turn_engine_on >( mbox );
				} );
		} );
}

void create_machine_controllers(
	so_5::rt::agent_coop_t & coop,
	const so_5::rt::mbox_t & status_distrib_mbox,
	const machine_dictionary_t & machines )
{
	// There must be a priority-respected dispatcher.
	auto disp = so_5::disp::prio::common_thread::create_private_disp(
			coop.environment() );

	// A helper lambda to reduce code repetition.
	auto maker = [&]( so_5::priority_t priority,
			a_machine_controller_t::filter_t filter,
			a_machine_controller_t::actor_t actor )
		{
			coop.make_agent_with_binder< a_machine_controller_t >(
					disp->binder(),
					priority,
					status_distrib_mbox,
					machines,
					std::move( filter ),
					std::move( actor ) );
		};

	// Creation of controllers.
	maker( so_5::prio::p4, engine_stopper_filter, engine_stopper_action );
	maker( so_5::prio::p3, cooler_starter_filter, cooler_starter_action );
	maker( so_5::prio::p2, engine_starter_filter, engine_starter_action );
	maker( so_5::prio::p1, cooler_stopper_filter, cooler_stopper_action );
}

void fill_coop( so_5::rt::agent_coop_t & coop )
{
	// Common mbox for information distribution.
	auto status_distrib_mbox = coop.environment().create_local_mbox();

	const auto & machine_dict = create_machines( coop, status_distrib_mbox );

	// Machine dashboard will work on its own dedicated thread.
	coop.make_agent_with_binder< a_total_status_dashboard_t >(
			so_5::disp::one_thread::create_private_disp(
					coop.environment() )->binder(),
			status_distrib_mbox );

	// Status analyzer will work on its own dedicated thread.
	coop.make_agent_with_binder< a_statuses_analyzer_t >(
			so_5::disp::one_thread::create_private_disp(
					coop.environment() )->binder(),
			status_distrib_mbox,
			50.0f, // Safe temperature.
			70.0f, // Warn temperature (cooler must be turned on)
			95.0f  // Critical temperature (engine must be turned off).
		);

	create_machine_controllers( coop, status_distrib_mbox, machine_dict );

	// Special agent which will start machines.
	create_starter_agent( coop, machine_dict );
}

void init( so_5::rt::environment_t & env )
{
	env.introduce_coop( fill_coop );
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

