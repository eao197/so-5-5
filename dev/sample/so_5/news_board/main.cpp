/*
 * An example for imitation of news board which handles requests from
 * different types of clients: news-writers and news-readers.
 */

#include <iostream>
#include <chrono>
#include <ctime>
#include <random>

#include <so_5/all.hpp>

// Helper function to generate a random integer in the specified range.
unsigned int
random_value( unsigned int left, unsigned int right )
	{
		return std::uniform_int_distribution< unsigned int >{ left, right }
			( std::mt19937{ std::random_device{}() } );
	}

// Type of clock to work with time values.
using clock_type = std::chrono::system_clock;

// A helper function to calculate difference between to time points.
std::uint64_t
ms_from_time( const clock_type::time_point & previous_point )
	{
		using namespace std::chrono;

		const auto t = clock_type::now();
		if( t > previous_point )
			return duration_cast< milliseconds >(
					clock_type::now() - previous_point ).count();
		else
			return 0;
	}

// A message for logging something.
struct msg_log : public so_5::rt::message_t
	{
		std::string m_who;
		std::string m_what;

		msg_log( std::string who, std::string what )
			:	m_who( std::move(who) )
			,	m_what( std::move(what) )
			{}
	};

// A helper for logging simplification.
void
log(
	const so_5::rt::mbox_t & logger_mbox,
	std::string who,
	std::string what )
	{
		so_5::send< msg_log >( logger_mbox, std::move(who), std::move(what) );
	}

// Builder of logger agent.
so_5::rt::mbox_t
create_logger_coop( so_5::rt::environment_t & env )
	{
		so_5::rt::mbox_t result;

		env.introduce_coop( [&]( so_5::rt::agent_coop_t & coop )
			{
				// Logger agent.
				auto a = coop.define_agent();
				// Reacts to just one message.
				a.event( a, []( const msg_log & evt ) {
					// String representation for date/time.
					char local_time_sz[ 32 ];
					auto t = clock_type::to_time_t( clock_type::now() );
					std::strftime( local_time_sz, sizeof local_time_sz,
							"%Y.%m.%d %H:%M:%S", std::localtime( &t ) );

					// Simplest form of logging.
					std::cout << "[" << local_time_sz << "] {" << evt.m_who
							<< "}: " << evt.m_what << std::endl;
				} );

				// Direct mbox of logger agent will be returned.
				result = a.direct_mbox();
			} );

		return result;
	}

//
// Messages for interaction with news board.
//

using story_id_type = unsigned long;

// Base class for all messages. It necessary to store timestamp.
struct news_board_message_base : public so_5::rt::message_t
	{
		const clock_type::time_point m_timestamp;

		news_board_message_base( clock_type::time_point timestamp )
			:	m_timestamp( std::move(timestamp) )
			{}
	};

// Request for publishing new story.
struct msg_publish_story_req : public news_board_message_base
	{
		const so_5::rt::mbox_t m_reply_to;
		const std::string m_title;
		const std::string m_content;

		msg_publish_story_req(
			clock_type::time_point timestamp,
			so_5::rt::mbox_t reply_to,
			std::string title,
			std::string content )
			:	news_board_message_base( std::move(timestamp) )
			,	m_reply_to( std::move(reply_to) )
			,	m_title( std::move(title) )
			,	m_content( std::move(content) )
			{}
	};

// Reply for publishing new story.
struct msg_publish_story_resp : public news_board_message_base
	{
		story_id_type m_id;

		msg_publish_story_resp(
			clock_type::time_point timestamp,
			story_id_type id )
			:	news_board_message_base( std::move(timestamp) )
			,	m_id( id )
			{}
	};

//
// News board data.
//

struct news_board_data
	{
		// Information about one story.
		struct story_info
			{
				std::string m_title;
				std::string m_content;
			};

		// Type of map from story ID to story data.
		using story_map = std::map< story_id_type, story_info >;

		// Published stories.
		story_map m_stories;

		// ID counter.
		story_id_type m_last_id = 0;
	};

//
// Agents to work with news board data.
//

void
define_news_receiver_agent(
	so_5::rt::agent_coop_t & coop,
	news_board_data & board_data,
	const so_5::rt::mbox_t & board_mbox,
	const so_5::rt::mbox_t & logger_mbox )
	{
		// Publisher should have lowest priority among board-related agents.
		coop.define_agent( coop.make_agent_context() + so_5::prio::p1 )
			// It handles just one message.
			.event( board_mbox,
				[&board_data, logger_mbox]( const msg_publish_story_req & evt )
				{
					// Store new story to board.
					auto story_id = ++(board_data.m_last_id);
					board_data.m_stories.emplace( story_id,
							news_board_data::story_info{ evt.m_title, evt.m_content } );

					// Log this fact.
					log( logger_mbox,
							"news_receiver",
							"new story published, id=" + std::to_string( story_id ) +
							", title=" + evt.m_title );

					// Send reply to story-sender.
					so_5::send< msg_publish_story_resp >(
							evt.m_reply_to,
							evt.m_timestamp,
							story_id );

					// Remove oldest story if there are too much stories.
					if( 40 < board_data.m_stories.size() )
						{
							auto removed_id = board_data.m_stories.begin()->first;
							board_data.m_stories.erase( board_data.m_stories.begin() );
							log( logger_mbox,
									"news_receiver",
									"old story removed, id=" + std::to_string( removed_id ) );
						}
				} );
	}

so_5::rt::mbox_t
create_board_coop(
	so_5::rt::environment_t & env,
	const so_5::rt::mbox_t & logger_mbox )
	{
		auto board_mbox = env.create_local_mbox();

		using namespace so_5::disp::prio_one_thread::quoted_round_robin;
		using namespace so_5::prio;

		// Board cooperation will use quoted_round_robin dispatcher
		// with different quotes for agents.
		env.introduce_coop(
			create_private_disp( env,
				quotes_t{ 1 }
					.set( p1, 10 ) // 10 events for news_receiver.
					.set( p2, 20 ) // 20 events for news_directory.
					.set( p3, 30 ) // 30 events for story_extractor.
				)->binder(),
			[&]( so_5::rt::agent_coop_t & coop )
			{
				// Lifetime of news board data will be controlled by cooperation.
				auto board_data = coop.take_under_control( new news_board_data() );

				define_news_receiver_agent(
						coop, *board_data, board_mbox, logger_mbox );
			} );

		return board_mbox;
	}

//
// Story publishers and news readers.
//

class story_publisher : public so_5::rt::agent_t
	{
		struct msg_time_for_new_story : public so_5::rt::signal_t {};

	public :
		story_publisher(
			context_t ctx,
			std::string publisher_name,
			so_5::rt::mbox_t board_mbox,
			so_5::rt::mbox_t logger_mbox )
			:	so_5::rt::agent_t( ctx )
			,	m_name( std::move(publisher_name) )
			,	m_board_mbox( std::move(board_mbox) )
			,	m_logger_mbox( std::move(logger_mbox) )
			{}

		virtual void
		so_define_agent() override
			{
				this >>= st_await_new_story;

				st_await_new_story.event< msg_time_for_new_story >(
						&story_publisher::evt_time_for_new_story );

				st_await_publish_response.event(
						&story_publisher::evt_publish_response );
			}

		virtual void
		so_evt_start() override
			{
				initiate_time_for_new_story_signal();
			}

	private :
		// The agent will wait 'msg_time_for_new_story' signal in this state.
		const so_5::rt::state_t st_await_new_story = so_make_state();
		// The agent will wait a response to publising request in this state.
		const so_5::rt::state_t st_await_publish_response = so_make_state();

		const std::string m_name;

		const so_5::rt::mbox_t m_board_mbox;
		const so_5::rt::mbox_t m_logger_mbox;

		// This counter will be used in store generation procedure.
		unsigned int m_stories_counter = 0;

		void
		initiate_time_for_new_story_signal()
			{
				so_5::send_delayed_to_agent< msg_time_for_new_story >(
						*this,
						std::chrono::milliseconds{ random_value( 100, 1500 ) } );
			}

		void
		evt_time_for_new_story()
			{
				// Create new story.
				auto story_number = ++m_stories_counter;
				std::string title = "A story from " + m_name + " #" +
						std::to_string( story_number );
				std::string content = "This is a content from a story '" +
						title + "' provided by " + m_name;

				log( m_logger_mbox, m_name, "Publish new story: " + title );

				// Publishing the story.
				so_5::send< msg_publish_story_req >( m_board_mbox,
						clock_type::now(),
						so_direct_mbox(),
						std::move(title),
						std::move(content) );

				// Waiting a response.
				this >>= st_await_publish_response;
			}

		void
		evt_publish_response( const msg_publish_story_resp & resp )
			{
				log( m_logger_mbox, m_name, "Publish finished, id=" +
						std::to_string( resp.m_id ) + ", publish took " +
						std::to_string( ms_from_time( resp.m_timestamp ) ) + "ms" );

				// Waiting for a time for next story.
				this >>= st_await_new_story;
				initiate_time_for_new_story_signal();
			}
	};

void
create_publisher_coop(
	so_5::rt::environment_t & env,
	const so_5::rt::mbox_t & board_mbox,
	const so_5::rt::mbox_t & logger_mbox )
	{
		// All publishers will work on the same working thread.
		env.introduce_coop(
			so_5::disp::one_thread::create_private_disp( env )->binder(),
			[&]( so_5::rt::agent_coop_t & coop )
			{
				for( int i = 0; i != 5; ++i )
					coop.make_agent< story_publisher >(
							"publisher" + std::to_string(i+1),
							board_mbox,
							logger_mbox );
			} );
	}

void
init( so_5::rt::environment_t & env )
	{
		auto logger_mbox = create_logger_coop( env );
		auto board_mbox = create_board_coop( env, logger_mbox );

		create_publisher_coop( env, board_mbox, logger_mbox );
	}

int
main()
	{
		try
			{
				so_5::launch( init );

				return 0;
			}
		catch( const std::exception & x )
			{
				std::cerr << "Exception: " << x.what() << std::endl;
			}

		return 2;
	}

