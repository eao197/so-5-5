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

//
// demand_t
//
/*!
 * \since v.5.5.13
 * \brief Description of one demand in message chain.
 */
struct demand_t
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
		demand_t()
			:	m_msg_type( typeid(void) )
			{}
		//! Initializing constructor.
		demand_t(
			std::type_index msg_type,
			so_5::rt::message_ref_t message_ref,
			so_5::rt::invocation_type_t demand_type )
			:	m_msg_type{ std::move(msg_type) }
			,	m_message_ref{ std::move(message_ref) }
			,	m_demand_type{ demand_type }
			{}

		//! Copy constructor.
		demand_t( const demand_t & o )
			:	m_msg_type{ o.m_msg_type }
			,	m_message_ref{ o.m_message_ref }
			,	m_demand_type{ o.m_demand_type }
			{}
		//! Move constructor.
		demand_t( demand_t && o )
			:	m_msg_type{ std::move(o.m_msg_type) }
			,	m_message_ref{ std::move(o.m_message_ref) }
			,	m_demand_type{ std::move(o.m_demand_type) }
			{}

		//! Swap operation.
		void
		swap( demand_t & o )
			{
				std::swap( m_msg_type, o.m_msg_type );
				m_message_ref.swap( o.m_message_ref );
				std::swap( m_demand_type, o.m_demand_type );
			}

		//! Copy operator.
		demand_t &
		operator=( const demand_t & o )
			{
				demand_t tmp{ o };
				tmp.swap( *this );
				return *this;
			}

		//! Move operator.
		demand_t &
		operator=( demand_t && o )
			{
				demand_t tmp{ std::move(o) };
				tmp.swap( *this );
				return *this;
			}
	};

using clock = std::chrono::high_resolution_clock;

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
// capacity_t
//
/*!
 * \since v.5.5.13
 * \brief Parameters for defining chain size.
 */
class capacity_t
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
		clock::duration m_overflow_timeout;

		//! Initializing constructor for size-limited message chain.
		capacity_t(
			std::size_t max_size,
			storage_memory memory,
			overflow_reaction overflow_reaction,
			clock::duration overflow_timeout )
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
		capacity_t()
			{}

		//! Create capacity description for size-unlimited message chain.
		inline static capacity_t
		make_unlimited() { return capacity_t{}; }

		//! Create capacity description for size-limited message chain
		//! without waiting on full queue during 'push message' operation.
		inline static capacity_t
		make_limited_without_waiting(
			//! Max size of the chain.
			std::size_t max_size,
			//! Type of chain storage.
			storage_memory memory,
			//! Reaction on chain overflow.
			overflow_reaction overflow_reaction )
			{
				return capacity_t{
						max_size,
						memory,
						overflow_reaction,
						clock::duration::zero()
				};
			}

		//! Create capacity description for size-limited message chain
		//! with waiting on full queue during 'push message' operation.
		inline static capacity_t
		make_limited_with_waiting(
			//! Max size of the chain.
			std::size_t max_size,
			//! Type of chain storage.
			storage_memory memory,
			//! Reaction on chain overflow.
			overflow_reaction overflow_reaction,
			//! Waiting time on full message chain.
			clock::duration wait_timeout )
			{
				return capacity_t{
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
				return clock::duration::zero() != m_overflow_timeout;
			}

		//! Get the value of waiting timeout for overflow case.
		/*!
		 * \attention Has sence only for size-limited chain.
		 */
		clock::duration
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
// abstract_message_chain_t
//
/*!
 * \since v.5.5.13
 * \brief An interace of message chain.
 */
class SO_5_TYPE abstract_message_chain_t : protected so_5::rt::abstract_message_box_t
	{
		friend class intrusive_ptr_t< abstract_message_chain_t >;

		abstract_message_chain_t( const abstract_message_chain_t & ) = delete;
		abstract_message_chain_t &
		operator=( const abstract_message_chain_t & ) = delete;

	protected :
		abstract_message_chain_t();
		virtual ~abstract_message_chain_t();

	public :
		virtual mchain_props::extraction_status
		extract(
			//! Destination for extracted messages.
			mchain_props::demand_t & dest,
			//! Max time to wait on empty queue.
			mchain_props::clock::duration empty_queue_timeout ) = 0;

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
// mchain_t
//
/*!
 * \since v.5.5.13
 * \brief Short name for smart pointer to message chain.
 */
using mchain_t = intrusive_ptr_t< abstract_message_chain_t >;

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
	so_5::mchain_t & ch = ...;
	... // Some work with chain.
	close_drop_content( ch );
	// Or:
	ch->close( so_5::mchain_props::close_mode::drop_content );
	\endcode
 */
inline void
close_drop_content( const mchain_t & ch )
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
	so_5::mchain_t & ch = ...;
	... // Some work with chain.
	close_retain_content( ch );
	// Or:
	ch->close( so_5::mchain_props::close_mode::retain_content );
	\endcode
 */
inline void
close_retain_content( const mchain_t & ch )
	{
		ch->close( mchain_props::close_mode::retain_content );
	}

//
// mchain_params_t
//
/*!
 * \since v.5.5.13
 * \brief Parameters for message chain.
 */
class mchain_params_t
	{
		//! Bag's capacity.
		mchain_props::capacity_t m_capacity;

	public :
		//! Initializing constructor.
		mchain_params_t(
			//! Bag's capacity and related params.
			mchain_props::capacity_t capacity )
			:	m_capacity{ capacity }
			{}

		//! Set chain's capacity and related params.
		mchain_params_t &
		capacity( mchain_props::capacity_t capacity )
			{
				m_capacity = capacity;
				return *this;
			}

		//! Get bag's capacity and related params.
		const mchain_props::capacity_t &
		capacity() const
			{
				return m_capacity;
			}
	};

//
// receive
//
//FIXME: Normal Doxygen comment must be written!
/*!
 * \since v.5.5.13
 * \brief Receive and handle one message from message chain.
 */
template< typename... HANDLERS >
inline std::size_t
receive(
	//! Message chain from which a message must be extracted.
	const so_5::mchain_t & chain,
	//! Maximum timeout for waiting for message on empty bag.
	mchain_props::clock::duration waiting_timeout,
	//! Handlers for message processing.
	HANDLERS &&... handlers )
	{
		using namespace so_5::rt::details;
		using namespace so_5::mchain_props;

		handlers_bunch_t< sizeof...(handlers) > bunch;
		fill_handlers_bunch( bunch, 0,
				std::forward< HANDLERS >(handlers)... );

		demand_t demand;
		if( extraction_status::msg_extracted ==
				chain->extract( demand, waiting_timeout ) )
			{
				bunch.handle(
						demand.m_msg_type,
						demand.m_message_ref,
						demand.m_demand_type );
				return 1;
			}

		return 0;
	}

} /* namespace so_5 */

