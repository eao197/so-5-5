/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Public part of message chain related stuff.
 */

#pragma once

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/handler_makers.hpp>

#include <chrono>

namespace so_5 {

namespace mchain_props {

/*!
 * \since v.5.5.13
 * \brief An alias for type for repesenting timeout values.
 */
using duration = std::chrono::high_resolution_clock::duration;

namespace details {

//
// no_wait_special_timevalue
//
/*!
 * \since v.5.5.13
 * \brief Special value of %duration to indicate 'no_wait' case.
 */
inline duration
no_wait_special_timevalue() { return duration::zero(); }

//
// infinite_wait_special_timevalue
//
/*!
 * \since v.5.5.13
 * \brief Special value of %duration to indicate 'infinite_wait' case.
 */
inline duration
infinite_wait_special_timevalue() { return duration::max(); }

//
// is_no_wait_timevalue
//
/*!
 * \since v.5.5.13
 * \brief Is time value means 'no_wait'?
 */
inline bool
is_no_wait_timevalue( duration v )
	{
		return v == no_wait_special_timevalue();
	}

//
// is_infinite_wait_timevalue
//
/*!
 * \since v.5.5.13
 * \brief Is time value means 'infinite_wait'?
 */
inline bool
is_infinite_wait_timevalue( duration v )
	{
		return v == infinite_wait_special_timevalue();
	}

//
// actual_timeout
//

/*!
 * \since v.5.5.13
 * \brief Helper function for detection of actual value for waiting timeout.
 *
 * \note This helper implements convention that infinite waiting is
 * represented as duration::max() value.
 */
inline duration
actual_timeout( infinite_wait_indication )
	{
		return infinite_wait_special_timevalue();
	}

/*!
 * \since v.5.5.13
 * \brief Helper function for detection of actual value for waiting timeout.
 *
 * \note This helper implements convention that no waiting is
 * represented as duration::zero() value.
 */
inline duration
actual_timeout( no_wait_indication )
	{
		return no_wait_special_timevalue();
	}

/*!
 * \since v.5.5.13
 * \brief Helper function for detection of actual value for waiting timeout.
 */
template< typename V >
duration
actual_timeout( V value )
	{
		return duration( value );
	}

} /* namespace details */

//
// demand
//
/*!
 * \since v.5.5.13
 * \brief Description of one demand in message chain.
 */
struct demand
	{
		//! Type of the message.
		std::type_index m_msg_type;
		//! Event incident.
		so_5::rt::message_ref_t m_message_ref;
		//! Type of demand.
		so_5::rt::invocation_type_t m_demand_type;

// NOTE: the full set of constructors and copy/move operators is defined
// because VC++12.0 doesn't generate move constructors/operators automatically
// and doesn't support '=default' construct.

		//! Default constructor.
		demand()
			:	m_msg_type( typeid(void) )
			{}
		//! Initializing constructor.
		demand(
			std::type_index msg_type,
			so_5::rt::message_ref_t message_ref,
			so_5::rt::invocation_type_t demand_type )
			:	m_msg_type{ std::move(msg_type) }
			,	m_message_ref{ std::move(message_ref) }
			,	m_demand_type{ demand_type }
			{}

		//! Copy constructor.
		demand( const demand& o )
			:	m_msg_type{ o.m_msg_type }
			,	m_message_ref{ o.m_message_ref }
			,	m_demand_type{ o.m_demand_type }
			{}
		//! Move constructor.
		demand( demand && o )
			:	m_msg_type{ std::move(o.m_msg_type) }
			,	m_message_ref{ std::move(o.m_message_ref) }
			,	m_demand_type{ std::move(o.m_demand_type) }
			{}

		//! Swap operation.
		void
		swap( demand & o )
			{
				std::swap( m_msg_type, o.m_msg_type );
				m_message_ref.swap( o.m_message_ref );
				std::swap( m_demand_type, o.m_demand_type );
			}

		//! Copy operator.
		demand &
		operator=( const demand & o )
			{
				demand tmp{ o };
				tmp.swap( *this );
				return *this;
			}

		//! Move operator.
		demand &
		operator=( demand && o )
			{
				demand tmp{ std::move(o) };
				tmp.swap( *this );
				return *this;
			}
	};

//FIXME: it is not a good name. A better name must be found
//(something like memory_consumption).
//
// storage_memory
//
/*!
 * \since v.5.5.13
 * \brief Memory allocation for storage for size-limited chains.
 */
enum class storage_memory
	{
		//! Storage can be allocated and deallocated dynamically.
		dynamic,
		//! Storage must be preallocated once and doesn't change after that.
		preallocated
	};

//
// overflow_reaction
//
/*!
 * \since v.5.5.13
 * \brief What reaction must be performed on attempt to push new message to
 * the full message chain.
 */
enum class overflow_reaction
	{
		//! Application must be aborted.
		abort_app,
		//! An exception must be thrown.
		throw_exception,
		//! New message must be ignored and droped.
		drop_newest,
		//! Oldest message in chain must be removed.
		remove_oldest
	};

//
// capacity
//
/*!
 * \since v.5.5.13
 * \brief Parameters for defining chain size.
 */
class capacity
	{
		//! Has chain unlimited size?
		bool m_unlimited = { true };

		// NOTE: all other atributes have sence only if m_unlimited != true.

		//! Max size of the chain with limited size.
		std::size_t m_max_size;

		//! Type of the storage for size-limited chain.
		storage_memory m_memory;

		//! Type of reaction for chain overflow.
		overflow_reaction m_overflow_reaction;

		//! Timeout for waiting on full chain during 'message push' operation.
		/*!
		 * \note Value 'zero' means that there must not be waiting on
		 * full chain.
		 */
		duration m_overflow_timeout;

		//! Initializing constructor for size-limited message chain.
		capacity(
			std::size_t max_size,
			storage_memory memory,
			overflow_reaction overflow_reaction,
			duration overflow_timeout )
			:	m_unlimited{ false }
			,	m_max_size{ max_size }
			,	m_memory{ memory }
			,	m_overflow_reaction{ overflow_reaction }
			,	m_overflow_timeout( overflow_timeout )
			{}

	public :
		//! Default constructor.
		/*!
		 * Creates description for size-unlimited chain.
		 */
		capacity()
			{}

		//! Create capacity description for size-unlimited message chain.
		inline static capacity
		make_unlimited() { return capacity{}; }

		//! Create capacity description for size-limited message chain
		//! without waiting on full queue during 'push message' operation.
		inline static capacity
		make_limited_without_waiting(
			//! Max size of the chain.
			std::size_t max_size,
			//! Type of chain storage.
			storage_memory memory,
			//! Reaction on chain overflow.
			overflow_reaction overflow_reaction )
			{
				return capacity{
						max_size,
						memory,
						overflow_reaction,
						details::no_wait_special_timevalue()
				};
			}

		//! Create capacity description for size-limited message chain
		//! with waiting on full queue during 'push message' operation.
		inline static capacity
		make_limited_with_waiting(
			//! Max size of the chain.
			std::size_t max_size,
			//! Type of chain storage.
			storage_memory memory,
			//! Reaction on chain overflow.
			overflow_reaction overflow_reaction,
			//! Waiting time on full message chain.
			duration wait_timeout )
			{
				return capacity{
						max_size,
						memory,
						overflow_reaction,
						wait_timeout
				};
			}

		//! Is message chain have no size limit?
		bool
		unlimited() const { return m_unlimited; }

		//! Max size for size-limited chain.
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		std::size_t
		max_size() const { return m_max_size; }

		//! Memory allocation type for size-limited chain.
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		storage_memory
		memory() const { return m_memory; }

		//! Overflow reaction for size-limited chain.
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		overflow_reaction
		overflow_reaction() const { return m_overflow_reaction; }

		//! Is waiting timeout for overflow case defined?
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		bool
		is_overflow_timeout_defined() const
			{
				return !details::is_no_wait_timevalue( m_overflow_timeout );
			}

		//! Get the value of waiting timeout for overflow case.
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		duration
		overflow_timeout() const
			{
				return m_overflow_timeout;
			}
	};

//
// extraction_status
//
/*!
 * \since v.5.5.13
 * \brief Result of extraction of message from a message chain.
 */
enum class extraction_status
	{
		//! No available messages in the chain.
		no_messages,
		//! Message extracted successfully.
		msg_extracted,
		//! Message cannot be extracted because chain is closed.
		chain_closed
	};

//
// close_mode
//
/*!
 * \since v.5.5.13
 * \brief What to do with chain's content at close.
 */
enum class close_mode
	{
		//! All messages must be removed from chain.
		drop_content,
		//! All messages must be retained until they will be
		//! processed at receiver's side.
		retain_content
	};

} /* namespace mchain_props */

//
// abstract_message_chain
//
/*!
 * \since v.5.5.13
 * \brief An interace of message chain.
 */
class SO_5_TYPE abstract_message_chain : protected so_5::rt::abstract_message_box_t
	{
		friend class intrusive_ptr_t< abstract_message_chain >;

		abstract_message_chain( const abstract_message_chain & ) = delete;
		abstract_message_chain &
		operator=( const abstract_message_chain & ) = delete;

	protected :
		abstract_message_chain();
		virtual ~abstract_message_chain();

	public :
		using abstract_message_box_t::id;

		virtual mchain_props::extraction_status
		extract(
			//! Destination for extracted messages.
			mchain_props::demand & dest,
			//! Max time to wait on empty queue.
			mchain_props::duration empty_queue_timeout ) = 0;

		//! Cast message chain to message box.
		so_5::rt::mbox_t
		as_mbox();

		//! Is message chain empty?
		virtual bool
		empty() const = 0;

		//! Count of messages in the chain.
		virtual std::size_t
		size() const = 0;

		//! Close the chain.
		virtual void
		close(
			//! What to do with chain's content.
			mchain_props::close_mode mode ) = 0;
	};

//
// mchain
//
/*!
 * \since v.5.5.13
 * \brief Short name for smart pointer to message chain.
 */
using mchain = intrusive_ptr_t< abstract_message_chain >;

//
// close_drop_content
//
/*!
 * \since v.5.5.13
 * \brief Helper function for closing a message chain with dropping
 * all its content.
 *
 * \note Because of ADL it can be used without specifying namespaces.
 *
 * \par Usage example.
	\code
	so_5::mchain & ch = ...;
	... // Some work with chain.
	close_drop_content( ch );
	// Or:
	ch->close( so_5::mchain_props::close_mode::drop_content );
	\endcode
 */
inline void
close_drop_content( const mchain & ch )
	{
		ch->close( mchain_props::close_mode::drop_content );
	}

//
// close_retain_content
//
/*!
 * \since v.5.5.13
 * \brief Helper function for closing a message chain with retaining
 * all its content.
 *
 * \note Because of ADL it can be used without specifying namespaces.
 *
 * \par Usage example.
	\code
	so_5::mchain & ch = ...;
	... // Some work with chain.
	close_retain_content( ch );
	// Or:
	ch->close( so_5::mchain_props::close_mode::retain_content );
	\endcode
 */
inline void
close_retain_content( const mchain & ch )
	{
		ch->close( mchain_props::close_mode::retain_content );
	}

//
// mchain_params
//
/*!
 * \since v.5.5.13
 * \brief Parameters for message chain.
 */
class mchain_params
	{
		//! Bag's capacity.
		mchain_props::capacity m_capacity;

	public :
		//! Initializing constructor.
		mchain_params(
			//! Bag's capacity and related params.
			mchain_props::capacity capacity )
			:	m_capacity{ capacity }
			{}

		//! Set chain's capacity and related params.
		mchain_params &
		capacity( mchain_props::capacity capacity )
			{
				m_capacity = capacity;
				return *this;
			}

		//! Get bag's capacity and related params.
		const mchain_props::capacity &
		capacity() const
			{
				return m_capacity;
			}
	};

/*!
 * \name Helper functions for creating parameters for %mchain.
 * \{
 */

/*!
 * \since v.5.5.13
 * \brief Create parameters for size-unlimited %mchain.
 *
 * \par Usage example:
	\code
	so_5::rt::environment_t & env = ...;
	auto chain = env.create_mchain( so_5::make_unlimited_mchain_params() );
	\endcode
 */
inline mchain_params
make_unlimited_mchain_params()
	{
		return mchain_params{ mchain_props::capacity::make_unlimited() };
	}

/*!
 * \since v.5.5.13
 * \brief Create parameters for size-limited %mchain without waiting on overflow.
 *
 * \par Usage example:
	\code
	so_5::rt::environment_t & env = ...;
	auto chain = env.create_mchain( so_5::make_limited_without_waiting_mchain_params(
			// No more than 200 messages in the chain.
			200,
			// Memory will be allocated dynamically.
			so_5::mchain_props::storage_memory::dynamic,
			// New messages will be ignored on chain's overflow.
			so_5::mchain_props::overflow_reaction::drop_newest ) );
	\endcode
 */
inline mchain_params
make_limited_without_waiting_mchain_params(
	//! Max capacity of %mchain.
	std::size_t max_size,
	//! Type of chain storage.
	mchain_props::storage_memory memory,
	//! Reaction on chain overflow.
	mchain_props::overflow_reaction overflow_reaction )
	{
		return mchain_params{
				mchain_props::capacity::make_limited_without_waiting(
						max_size,
						memory,
						overflow_reaction )
		};
	}

/*!
 * \since v.5.5.13
 * \brief Create parameters for size-limited %mchain with waiting on overflow.
 *
 * \par Usage example:
	\code
	so_5::rt::environment_t & env = ...;
	auto chain = env.create_mchain( so_5::make_limited_with_waiting_mchain_params(
			// No more than 200 messages in the chain.
			200,
			// Memory will be preallocated.
			so_5::mchain_props::storage_memory::preallocated,
			// New messages will be ignored on chain's overflow.
			so_5::mchain_props::overflow_reaction::drop_newest,
			// But before dropping a new message there will be 500ms timeout
			std::chrono::milliseconds(500) ) );
	\endcode
 */
inline mchain_params
make_limited_with_waiting_mchain_params(
	//! Max size of the chain.
	std::size_t max_size,
	//! Type of chain storage.
	mchain_props::storage_memory memory,
	//! Reaction on chain overflow.
	mchain_props::overflow_reaction overflow_reaction,
	//! Waiting time on full message chain.
	mchain_props::duration wait_timeout )
	{
		return mchain_params {
				mchain_props::capacity::make_limited_with_waiting(
						max_size,
						memory,
						overflow_reaction,
						wait_timeout )
		};
	}

/*!
 * \}
 */

//
// mchain_receive_result
//
/*!
 * \since v.5.5.13
 * \brief A result of receive from %mchain.
 */
class mchain_receive_result
	{
		//! Count of extracted messages.
		std::size_t m_extracted;
		//! Count of handled messages.
		std::size_t m_handled;
		//! Extraction status (e.g. no messages, chain closed and so on).
		mchain_props::extraction_status m_status;

	public :
		//! Default constructor.
		mchain_receive_result()
			:	m_extracted{ 0 }
			,	m_handled{ 0 }
			,	m_status{ mchain_props::extraction_status::no_messages }
			{}

		//! Initializing constructor.
		mchain_receive_result(
			//! Count of extracted messages.
			std::size_t extracted,
			//! Count of handled messages.
			std::size_t handled,
			//! Status of extraction operation.
			mchain_props::extraction_status status )
			:	m_extracted{ extracted }
			,	m_handled{ handled }
			,	m_status{ status }
			{}

		//! Count of extracted messages.
		std::size_t
		extracted() const { return m_extracted; }

		//! Count of handled messages.
		std::size_t
		handled() const { return m_handled; }

		//! Extraction status (e.g. no messages, chain closed and so on).
		mchain_props::extraction_status
		status() const { return m_status; }
	};

//
// receive
//
//FIXME: Normal Doxygen comment must be written!
/*!
 * \since v.5.5.13
 * \brief Receive and handle one message from message chain.
 */
template< typename TIMEOUT, typename... HANDLERS >
inline mchain_receive_result
receive(
	//! Message chain from which a message must be extracted.
	const so_5::mchain & chain,
	//! Maximum timeout for waiting for message on empty bag.
	TIMEOUT waiting_timeout,
	//! Handlers for message processing.
	HANDLERS &&... handlers )
	{
		using namespace so_5::rt::details;
		using namespace so_5::mchain_props;
		using namespace so_5::mchain_props::details;

		handlers_bunch< sizeof...(handlers) > bunch;
		fill_handlers_bunch( bunch, 0,
				std::forward< HANDLERS >(handlers)... );

		demand extracted_demand;
		const auto status = chain->extract(
				extracted_demand,
				actual_timeout( waiting_timeout ) );
		if( extraction_status::msg_extracted == status )
			{
				const bool handled = bunch.handle(
						extracted_demand.m_msg_type,
						extracted_demand.m_message_ref,
						extracted_demand.m_demand_type );

				return mchain_receive_result{ 1u, handled ? 1u : 0u, status };
			}

		return mchain_receive_result{ 0u, 0u, status };
	}

//
// mchain_receive_params
//
//FIXME: Examples must be provided in Doxygen comment!
/*!
 * \since v.5.5.13
 * \brief Parameters for advanced receive from %mchain.
 */
class mchain_receive_params
	{
	public :
		//! Type of stop-predicate.
		/*!
		 * Must return \a true if receive procedure should be stopped.
		 */
		using stop_predicate = std::function< bool() >;

	private :
		//! Chain from which messages must be extracted and handled.
		mchain m_chain;

		//! Minimal count of messages to be extracted.
		/*!
		 * Value 0 means that this parameter is not set.
		 */
		std::size_t m_to_extract = { 0 };
		//! Minimal count of messages to be handled.
		/*!
		 * Value 0 means that this parameter it not set.
		 */
		std::size_t m_to_handle = { 0 };

		//! Timeout for waiting on empty queue.
		mchain_props::duration m_empty_timeout =
				{ mchain_props::details::infinite_wait_special_timevalue() };

		//! Total time for all work of advanced receive.
		mchain_props::duration m_total_time =
				{ mchain_props::details::infinite_wait_special_timevalue() };

		//! Optional stop-predicate.
		stop_predicate m_stop_predicate;

	public :
		//! Initializing constructor.
		mchain_receive_params(
			//! Chain from which messages must be extracted and handled.
			mchain chain )
			:	m_chain{ std::move(chain) }
			{}

		//! Chain from which messages must be extracted and handled.
		const mchain &
		chain() const { return m_chain; }

		//! Set limit for count of messages to be extracted.
		mchain_receive_params &
		extract_n( std::size_t v )
			{
				m_to_extract = v;
				return *this;
			}

		//! Get limit for count of messages to be extracted.
		std::size_t
		to_extract() const { return m_to_extract; }

		//! Set limit for count of messages to be handled.
		mchain_receive_params &
		handle_n( std::size_t v )
			{
				m_to_handle = v;
				return *this;
			}

		//! Get limit for count of message to be handled.
		std::size_t
		to_handle() const { return m_to_handle; }

		//! Set timeout for waiting on empty chain.
		/*!
		 * \note This value will be ignored if total_time() is also used
		 * to set total receive time.
		 *
		 * \note Argument \a v can be of type duration or
		 * so_5::infinite_wait or so_5::no_wait.
		 */
		template< typename TIMEOUT >
		mchain_receive_params &
		empty_timeout( TIMEOUT v )
			{
				m_empty_timeout = mchain_props::details::actual_timeout( v );
				return *this;
			}

		//! Get timeout for waiting on empty chain.
		const mchain_props::duration &
		empty_timeout() const { return m_empty_timeout; }

		//! Set total time for the whole receive operation.
		/*!
		 * \note Argument \a v can be of type duration or
		 * so_5::infinite_wait or so_5::no_wait.
		 */
		template< typename TIMEOUT >
		mchain_receive_params &
		total_time( TIMEOUT v )
			{
				m_total_time = mchain_props::details::actual_timeout( v );
				return *this;
			}

		//! Get total time for the whole receive operation.
		const mchain_props::duration &
		total_time() const { return m_total_time; }

		//! Set user condition for stopping receive operation.
		/*!
		 * \note \a predicate should return \a true if receive must
		 * be stopped.
		 */
		mchain_receive_params &
		stop_on( stop_predicate predicate )
			{
				m_stop_predicate = std::move(predicate);
				return *this;
			}

		//! Get user condition for stopping receive operation.
		const stop_predicate &
		stop_on() const
			{
				return m_stop_predicate;
			}
	};

//
// from
//
//FIXME: Examples must be provided in Doxygen comment!
/*!
 * \since v.5.5.13
 * \brief A helper function for simplification of creation of %mchain_receive_params instance.
 */
inline mchain_receive_params
from( mchain chain )
	{
		return mchain_receive_params{ std::move(chain) };
	}

namespace mchain_props {

namespace details {

//
// receive_actions_performer
//
/*!
 * \since v.5.5.13
 * \brief Helper class with implementation of main actions of
 * advanced receive operation.
 */
template< typename BUNCH >
class receive_actions_performer
	{
		const mchain_receive_params & m_params;
		const BUNCH & m_bunch;

		std::size_t m_extracted_messages = 0;
		std::size_t m_handled_messages = 0;
		extraction_status m_status;

	public :
		receive_actions_performer(
			const mchain_receive_params & params,
			const BUNCH & bunch )
			:	m_params{ params }
			,	m_bunch{ bunch }
			{}

		void
		handle_next( duration empty_timeout )
			{
				demand extracted_demand;
				m_status = m_params.chain()->extract(
						extracted_demand, empty_timeout );

				if( extraction_status::msg_extracted == m_status )
					{
						++m_extracted_messages;
						const bool handled = m_bunch.handle(
								extracted_demand.m_msg_type,
								extracted_demand.m_message_ref,
								extracted_demand.m_demand_type );
						if( handled )
							++m_handled_messages;
					}
			}

		extraction_status
		last_status() const { return m_status; }

		bool
		can_continue() const
			{
				if( extraction_status::chain_closed == m_status )
					return false;

				if( m_params.to_handle() &&
						m_handled_messages >= m_params.to_handle() )
					return false;

				if( m_params.to_extract() &&
						m_extracted_messages >= m_params.to_extract() )
					return false;

				if( m_params.stop_on() && m_params.stop_on()() )
					return false;

				return true;
			}

		mchain_receive_result
		make_result() const
			{
				return mchain_receive_result{
						m_extracted_messages,
						m_handled_messages,
						m_extracted_messages ? extraction_status::msg_extracted :
								m_status
					};
			}
	};

/*!
 * \since v.5.5.13
 * \brief An implementation of advanced receive when a limit for total
 * operation time is defined.
 */
template< typename BUNCH >
inline mchain_receive_result
receive_with_finite_total_time(
	const mchain_receive_params & params,
	const BUNCH & bunch )
	{
		receive_actions_performer< BUNCH > performer{ params, bunch };

		duration remaining_time = params.total_time();
		const auto start_point = std::chrono::steady_clock::now();

		do
			{
				performer.handle_next( remaining_time );
				if( !performer.can_continue() )
					break;

				const auto elapsed = std::chrono::steady_clock::now() - start_point;
				if( elapsed < remaining_time )
					remaining_time -= elapsed;
				else
					remaining_time = duration::zero();
			}
		while( remaining_time > duration::zero() );

		return performer.make_result();
	}

/*!
 * \since v.5.5.13
 * \brief An implementation of advanced receive when there is no
 * limit for total operation time is defined.
 */
template< typename BUNCH >
inline mchain_receive_result
receive_without_total_time(
	const mchain_receive_params & params,
	const BUNCH & bunch )
	{
		receive_actions_performer< BUNCH > performer{ params, bunch };

		do
			{
				performer.handle_next( params.empty_timeout() );

				if( extraction_status::no_messages == performer.last_status() )
					// There is no need to continue.
					// This status means that empty_timeout has some value
					// and there is no any new message during empty_timeout.
					// And this means a condition for return from advanced
					// receive.
					break;
			}
		while( performer.can_continue() );

		return performer.make_result();
	}

} /* namespace details */

} /* namespace mchain_props */

//
// receve (advanced version)
//

//FIXME: Examples must be provided in Doxygen comment!
/*!
 * \since v.5.5.13
 * \brief Advanced version of receive from %mchain.
 */
template< typename... HANDLERS >
inline mchain_receive_result
receive(
	//! Parameters for receive.
	const mchain_receive_params & params,
	//! Handlers for message processing.
	HANDLERS &&... handlers )
	{
		using namespace so_5::rt::details;
		using namespace so_5::mchain_props;
		using namespace so_5::mchain_props::details;

		handlers_bunch< sizeof...(handlers) > bunch;
		fill_handlers_bunch( bunch, 0,
				std::forward< HANDLERS >(handlers)... );

		if( !is_infinite_wait_timevalue( params.total_time() ) )
			return receive_with_finite_total_time( params, bunch );
		else
			return receive_without_total_time( params, bunch );
	}

} /* namespace so_5 */

