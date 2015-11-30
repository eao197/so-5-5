/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Implementation details for message chains.
 */

#pragma once

#include <so_5/rt/h/mchain.hpp>
#include <so_5/rt/h/environment.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/exception.hpp>
#include <so_5/h/error_logger.hpp>

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace so_5 {

namespace mchain_props {

namespace details {

//
// ensure_queue_not_empty
//
/*!
 * \since v.5.5.13
 * \brief Helper function which throws an exception if queue is empty.
 */
template< typename Q >
void
ensure_queue_not_empty( Q && queue )
	{
		if( queue.is_empty() )
			SO_5_THROW_EXCEPTION(
					rc_msg_chain_is_empty,
					"an attempt to get message from empty demand queue" );
	}

//
// ensure_queue_not_full
//
/*!
 * \since v.5.5.13
 * \brief Helper function which throws an exception if queue is full.
 */
template< typename Q >
void
ensure_queue_not_full( Q && queue )
	{
		if( queue.is_full() )
			SO_5_THROW_EXCEPTION(
					rc_msg_chain_is_full,
					"an attempt to push a message to full demand queue" );
	}

//
// unlimited_demand_queue
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-unlimited message chain.
 */
class unlimited_demand_queue
	{
	public :
		/*!
		 * \note This constructor is necessary just for a convinience.
		 */
		unlimited_demand_queue( const capacity & ) {}

		//! Is queue full?
		/*!
		 * \note Unlimited queue can't be null. Because of that this
		 * method always returns \a false.
		 */
		bool
		is_full() const { return false; }

		//! Is queue empty?
		bool
		is_empty() const { return m_queue.empty(); }

		//! Access to front item from queue.
		demand &
		front()
			{
				ensure_queue_not_empty( *this );
				return m_queue.front();
			}

		//! Remove the front item from queue.
		void
		pop_front()
			{
				ensure_queue_not_empty( *this );
				m_queue.pop_front();
			}

		//! Add a new item to the end of the queue.
		void
		push_back( demand && demand )
			{
				m_queue.push_back( std::move(demand) );
			}

		//! Size of the queue.
		std::size_t
		size() const { return m_queue.size(); }

	private :
		//! Queue's storage.
		std::deque< demand > m_queue;
	};

//
// limited_dynamic_demand_queue
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-limited message chain with
 * dynamically allocated storage.
 */
class limited_dynamic_demand_queue
	{
	public :
		//! Initializing constructor.
		limited_dynamic_demand_queue(
			const capacity & capacity )
			:	m_max_size{ capacity.max_size() }
			{}

		//! Is queue full?
		bool
		is_full() const { return m_max_size == m_queue.size(); }

		//! Is queue empty?
		bool
		is_empty() const { return m_queue.empty(); }

		//! Access to front item from queue.
		demand &
		front()
			{
				ensure_queue_not_empty( *this );
				return m_queue.front();
			}

		//! Remove the front item from queue.
		void
		pop_front()
			{
				ensure_queue_not_empty( *this );
				m_queue.pop_front();
			}

		//! Add a new item to the end of the queue.
		void
		push_back( demand && demand )
			{
				ensure_queue_not_full( *this );
				m_queue.push_back( std::move(demand) );
			}

		//! Size of the queue.
		std::size_t
		size() const { return m_queue.size(); }

	private :
		//! Queue's storage.
		std::deque< demand > m_queue;
		//! Maximum size of the queue.
		const std::size_t m_max_size;
	};

//
// limited_preallocated_demand_queue
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-limited message chain with
 * preallocated storage.
 */
class limited_preallocated_demand_queue
	{
	public :
		//! Initializing constructor.
		limited_preallocated_demand_queue(
			const capacity & capacity )
			:	m_storage( capacity.max_size(), demand{} )
			,	m_max_size{ capacity.max_size() }
			,	m_head{ 0 }
			,	m_size{ 0 }
			{}

		//! Is queue full?
		bool
		is_full() const { return m_max_size == m_size; }

		//! Is queue empty?
		bool
		is_empty() const { return 0 == m_size; }

		//! Access to front item from queue.
		demand &
		front()
			{
				ensure_queue_not_empty( *this );
				return m_storage[ m_head ];
			}

		//! Remove the front item from queue.
		void
		pop_front()
			{
				ensure_queue_not_empty( *this );
				m_storage[ m_head ] = demand{};
				m_head = (m_head + 1) % m_max_size;
				--m_size;
			}

		//! Add a new item to the end of the queue.
		void
		push_back( demand && demand )
			{
				ensure_queue_not_full( *this );
				auto index = (m_head + m_size) % m_max_size;
				m_storage[ index ] = std::move(demand);
				++m_size;
			}

		//! Size of the queue.
		std::size_t
		size() const { return m_size; }

	private :
		//! Queue's storage.
		std::vector< demand > m_storage;
		//! Maximum size of the queue.
		const std::size_t m_max_size;

		//! Index of the queue head.
		std::size_t m_head;
		//! The current size of the queue.
		std::size_t m_size;
	};

//
// status
//
/*!
 * \since v.5.5.13
 * \brief Status of the message chain.
 */
enum class status
	{
		//! Bag is open and can be used for message sending.
		open,
		//! Bag is closed. New messages cannot be sent to it.
		closed
	};

} /* namespace details */

//
// mchain_template
//
/*!
 * \since v.5.5.13
 * \brief Template-based implementation of message chain.
 *
 * \tparam QUEUE type of demand queue for message chain.
 */
template< typename QUEUE >
class mchain_template : public abstract_message_chain
	{
	public :
		//! Initializing constructor.
		mchain_template(
			//! SObjectizer Environment for which message chain is created.
			so_5::rt::environment_t & env,
			//! Mbox ID for this chain.
			mbox_id_t id,
			//! Bag's capacity.
			const capacity & capacity )
			:	m_env{ env }
			,	m_id{ id }
			,	m_capacity{ capacity }
			,	m_queue{ capacity }
			{}

		virtual mbox_id_t
		id() const override
			{
				return m_id;
			}

		virtual void
		subscribe_event_handler(
			const std::type_index & /*msg_type*/,
			const so_5::rt::message_limit::control_block_t * /*limit*/,
			so_5::rt::agent_t * /*subscriber*/ ) override
			{
				SO_5_THROW_EXCEPTION(
						rc_msg_chain_doesnt_support_subscriptions,
						"mchain doesn't suppor subscription" );
			}

		virtual void
		unsubscribe_event_handlers(
			const std::type_index & /*msg_type*/,
			so_5::rt::agent_t * /*subscriber*/ ) override
			{}

		virtual std::string
		query_name() const override
			{
				std::ostringstream s;
				s << "<mchain:id=" << m_id << ">";

				return s.str();
			}

		virtual so_5::rt::mbox_type_t
		type() const override
			{
				return so_5::rt::mbox_type_t::multi_producer_single_consumer;
			}

		virtual void
		do_deliver_message(
			const std::type_index & msg_type,
			const so_5::rt::message_ref_t & message,
			unsigned int /*overlimit_reaction_deep*/ ) const override
			{
				try_to_store_message_to_queue(
						msg_type,
						message,
						so_5::rt::invocation_type_t::event );
			}

		virtual void
		do_deliver_service_request(
			const std::type_index & msg_type,
			const so_5::rt::message_ref_t & message,
			unsigned int /*overlimit_reaction_deep*/ ) const override
			{
				try_to_store_message_to_queue(
						msg_type,
						message,
						so_5::rt::invocation_type_t::service_request );
			}

		/*!
		 * \attention Will throw an exception because delivery
		 * filter is not applicable to MPSC-mboxes.
		 */
		virtual void
		set_delivery_filter(
			const std::type_index & /*msg_type*/,
			const so_5::rt::delivery_filter_t & /*filter*/,
			so_5::rt::agent_t & /*subscriber*/ ) override
			{
				SO_5_THROW_EXCEPTION(
						rc_msg_chain_doesnt_support_delivery_filters,
						"set_delivery_filter is called for mchain" );
			}

		virtual void
		drop_delivery_filter(
			const std::type_index & /*msg_type*/,
			so_5::rt::agent_t & /*subscriber*/ ) SO_5_NOEXCEPT override
			{}

		virtual extraction_status
		extract(
			demand & dest,
			duration empty_queue_timeout ) override
			{
				std::unique_lock< std::mutex > lock{ m_lock };

				// If queue is empty we must wait for some time.
				bool queue_empty = m_queue.is_empty();
				if( queue_empty )
					{
						if( details::status::closed == m_status )
							// Waiting for new messages has no sence because
							// chain is closed.
							return extraction_status::chain_closed;

						auto predicate = [this, &queue_empty]() -> bool {
								queue_empty = m_queue.is_empty();
								return !queue_empty ||
										details::status::closed == m_status;
							};

						if( !details::is_infinite_wait_timevalue( empty_queue_timeout ) )
							// A wait with finite timeout must be performed.
							m_underflow_cond.wait_for(
									lock, empty_queue_timeout, predicate );
						else
							// Wait until arrival of any message or closing of chain.
							m_underflow_cond.wait( lock, predicate );
					}

				// If queue is still empty nothing can be extracted and
				// we must stop operation.
				if( queue_empty )
					return details::status::open == m_status ?
							// The chain is still open so there must be this result
							extraction_status::no_messages :
							// The chain is closed and there must be different result
							extraction_status::chain_closed;

				// If queue was full then someone can wait on it.
				const bool queue_was_full = m_queue.is_full();
				dest = std::move( m_queue.front() );
				m_queue.pop_front();

				if( queue_was_full )
					m_overflow_cond.notify_all();

				return extraction_status::msg_extracted;
			}

		virtual bool
		empty() const override
			{
				return m_queue.is_empty();
			}

		virtual std::size_t
		size() const override
			{
				return m_queue.size();
			}

		virtual void
		close( close_mode mode ) override
			{
				std::lock_guard< std::mutex > lock{ m_lock };

				if( details::status::closed == m_status )
					return;

				m_status = details::status::closed;

				const bool was_full = m_queue.is_full();

				if( close_mode::drop_content == mode )
					{
						const bool was_empty = m_queue.is_empty();
						while( !m_queue.is_empty() )
							m_queue.pop_front();

						if( was_empty )
							// Someone can wait on empty chain for new messages.
							// It must be informed that no new messages will be here.
							m_underflow_cond.notify_all();
					}

				if( was_full )
					// Someone can wait on full chain for free place for new message.
					// It must be informed that the chain is closed.
					m_overflow_cond.notify_all();
			}

	private :
		//! SObjectizer Environment for which message chain is created.
		so_5::rt::environment_t & m_env;

		//! Status of the chain.
		details::status m_status = { details::status::open };

		//! Mbox ID for chain.
		const mbox_id_t m_id;

		//! Bag capacity.
		const capacity m_capacity;

		//! Bag's demands queue.
		mutable QUEUE m_queue;

		//! Bag's lock.
		mutable std::mutex m_lock;

		//! Condition variable for waiting on empty queue.
		mutable std::condition_variable m_underflow_cond;
		//! Condition variable for waiting on full queue.
		mutable std::condition_variable m_overflow_cond;

		//! Actual implementation of pushing message to the queue.
		/*!
		 * \attention This method is marked as 'const' but it changes
		 * state of the object. It is because this method is called
		 * from do_deliver_message() and do_deliver_service_request() which
		 * must be 'const'. It is a flaw in the mbox'es design but
		 * this flaw must be fixed at different level (such as modification
		 * of abstract_message_box_t interface).
		 */
		void
		try_to_store_message_to_queue(
			const std::type_index & msg_type,
			const so_5::rt::message_ref_t & message,
			so_5::rt::invocation_type_t demand_type ) const
			{
				std::unique_lock< std::mutex > lock{ m_lock };

				// Message cannot be stored to closed chain.
				if( details::status::closed == m_status )
					return;

				// If queue full and waiting on full queue is enabled we
				// must wait for some time until there will be some space in
				// the queue.
				bool queue_full = m_queue.is_full();
				if( queue_full && m_capacity.is_overflow_timeout_defined() )
					{
						m_overflow_cond.wait_for(
								lock,
								m_capacity.overflow_timeout(),
								[this, &queue_full] {
									queue_full = m_queue.is_full();
									return !queue_full ||
											details::status::closed == m_status;
								} );
					}

				// If queue still full we must perform some reaction.
				if( queue_full )
					{
						const auto reaction = m_capacity.overflow_reaction();
						if( overflow_reaction::drop_newest == reaction )
							{
								// New message must be simply ignored.
								return;
							}
						else if( overflow_reaction::remove_oldest == reaction )
							{
								// The oldest message must be simply removed.
								m_queue.pop_front();
							}
						else if( overflow_reaction::throw_exception == reaction )
							{
								SO_5_THROW_EXCEPTION(
										rc_msg_chain_overflow,
										"an attempt to push message to full mchain "
										"with overflow_reaction::throw_exception policy" );
							}
						else
							{
								so_5::details::abort_on_fatal_error( [&] {
										SO_5_LOG_ERROR( m_env, log_stream ) {
											log_stream << "overflow_reaction::abort_app "
													"will be performed for mchain (id="
													<< m_id << "), msg_type: "
													<< msg_type.name()
													<< ". Application will be aborted"
													<< std::endl;
										}
									} );
							}
					}

				// May be someone is waiting on empty queue?
				const bool queue_was_empty = m_queue.is_empty();
				
				m_queue.push_back(
						demand{ msg_type, message, demand_type } );

				if( queue_was_empty )
					m_underflow_cond.notify_one();
			}
	};

} /* namespace mchain_props */

} /* namespace so_5 */

