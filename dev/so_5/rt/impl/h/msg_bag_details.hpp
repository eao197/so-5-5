/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Implementation details for message bags.
 */

#pragma once

#include <so_5/rt/h/msg_bag.hpp>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/exception.hpp>

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace so_5 {

namespace rt {

namespace msg_bag {

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
					rc_msg_bag_is_empty,
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
					rc_msg_bag_is_full,
					"an attempt to push a message to full demand queue" );
	}

//
// unlimited_demand_queue_t
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-unlimited message bag.
 */
class unlimited_demand_queue_t
	{
	public :
		/*!
		 * \note This constructor is necessary just for a convinience.
		 */
		unlimited_demand_queue_t( const capacity_t & ) {}

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
		bag_demand_t &
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
		push_back( bag_demand_t && demand )
			{
				m_queue.push_back( std::move(demand) );
			}

		//! Size of the queue.
		std::size_t
		size() const { return m_queue.size(); }

	private :
		//! Queue's storage.
		std::deque< bag_demand_t > m_queue;
	};

//
// limited_dynamic_demand_queue_t
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-limited message bag with
 * dynamically allocated storage.
 */
class limited_dynamic_demand_queue_t
	{
	public :
		//! Initializing constructor.
		limited_dynamic_demand_queue_t(
			const capacity_t & capacity )
			:	m_max_size{ capacity.max_size() }
			{}

		//! Is queue full?
		bool
		is_full() const { return m_max_size == m_queue.size(); }

		//! Is queue empty?
		bool
		is_empty() const { return m_queue.empty(); }

		//! Access to front item from queue.
		bag_demand_t &
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
		push_back( bag_demand_t && demand )
			{
				ensure_queue_not_full( *this );
				m_queue.push_back( std::move(demand) );
			}

		//! Size of the queue.
		std::size_t
		size() const { return m_queue.size(); }

	private :
		//! Queue's storage.
		std::deque< bag_demand_t > m_queue;
		//! Maximum size of the queue.
		const std::size_t m_max_size;
	};

//
// limited_preallocated_demand_queue_t
//
/*!
 * \since v.5.5.13
 * \brief Implementation of demands queue for size-limited message bag with
 * preallocated storage.
 */
class limited_preallocated_demand_queue_t
	{
	public :
		//! Initializing constructor.
		limited_preallocated_demand_queue_t(
			const capacity_t & capacity )
			:	m_storage( capacity.max_size(), bag_demand_t{} )
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
		bag_demand_t &
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
				m_storage[ m_head ] = bag_demand_t{};
				m_head = (m_head + 1) % m_max_size;
				--m_size;
			}

		//! Add a new item to the end of the queue.
		void
		push_back( bag_demand_t && demand )
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
		std::vector< bag_demand_t > m_storage;
		//! Maximum size of the queue.
		const std::size_t m_max_size;

		//! Index of the queue head.
		std::size_t m_head;
		//! The current size of the queue.
		std::size_t m_size;
	};

} /* namespace details */

//
// msg_bag_template_t
//
/*!
 * \since v.5.5.13
 * \brief Template-based implementation of message bag.
 *
 * \tparam QUEUE type of demand queue for message bag.
 */
template< typename QUEUE >
class msg_bag_template_t : public abstract_message_bag_t
	{
	public :
		//! Initializing constructor.
		msg_bag_template_t(
			//! Mbox ID for this bag.
			mbox_id_t id,
			//! Bag's capacity.
			const capacity_t & capacity )
			:	m_id{ id }
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
			agent_t * /*subscriber*/ ) override
			{
				SO_5_THROW_EXCEPTION(
						rc_msg_bag_doesnt_support_subscriptions,
						"msg_bag doesn't suppor subscription" );
			}

		virtual void
		unsubscribe_event_handlers(
			const std::type_index & /*msg_type*/,
			agent_t * /*subscriber*/ ) override
			{}

		virtual std::string
		query_name() const override
			{
				std::ostringstream s;
				s << "<msgbag:id=" << m_id << ">";

				return s.str();
			}

		virtual mbox_type_t
		type() const override
			{
				return mbox_type_t::multi_producer_single_consumer;
			}

		virtual void
		do_deliver_message(
			const std::type_index & msg_type,
			const message_ref_t & message,
			unsigned int /*overlimit_reaction_deep*/ ) const override
			{
				auto t = const_cast< msg_bag_template_t * >(this);
				t->try_to_store_message_to_queue(
						msg_type,
						message,
						invocation_type_t::event );
			}

		virtual void
		do_deliver_service_request(
			const std::type_index & msg_type,
			const message_ref_t & message,
			unsigned int /*overlimit_reaction_deep*/ ) const override
			{
				auto t = const_cast< msg_bag_template_t * >(this);
				t->try_to_store_message_to_queue(
						msg_type,
						message,
						invocation_type_t::service_request );
			}

		/*!
		 * \attention Will throw an exception because delivery
		 * filter is not applicable to MPSC-mboxes.
		 */
		virtual void
		set_delivery_filter(
			const std::type_index & /*msg_type*/,
			const delivery_filter_t & /*filter*/,
			agent_t & /*subscriber*/ ) override
			{
				SO_5_THROW_EXCEPTION(
						rc_msg_bag_doesnt_support_delivery_filters,
						"set_delivery_filter is called for msg_bag" );
			}

		virtual void
		drop_delivery_filter(
			const std::type_index & /*msg_type*/,
			agent_t & /*subscriber*/ ) SO_5_NOEXCEPT override
			{}

		virtual extraction_result_t
		extract(
			bag_demand_t & dest,
			clock::duration empty_queue_timeout ) override
			{
				std::unique_lock< std::mutex > lock{ m_lock };

				// If queue is empty we must wait for some time.
				bool queue_empty = m_queue.is_empty();
				if( queue_empty )
					{
						m_underflow_cond.wait_for( lock, empty_queue_timeout,
							[this, &queue_empty] {
								return !(queue_empty = m_queue.is_empty());
							} );
					}
				// If queue is still empty nothing can be extracted and
				// we must stop operation.
				if( queue_empty )
					return extraction_result_t::no_messages;

				// If queue was full then someone can wait on it.
				const bool queue_was_full = m_queue.is_full();
				dest = std::move( m_queue.front() );
				m_queue.pop_front();

				if( queue_was_full )
					m_overflow_cond.notify_all();

				return extraction_result_t::msg_extracted;
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

	private :
		//! Mbox ID for bag.
		const mbox_id_t m_id;

		//! Bag capacity.
		const capacity_t m_capacity;

		//! Bag's demands queue.
		QUEUE m_queue;

		//! Bag's lock.
		std::mutex m_lock;

		//! Condition variable for waiting on empty queue.
		std::condition_variable m_underflow_cond;
		//! Condition variable for waiting on full queue.
		std::condition_variable m_overflow_cond;

		//! Actual implementation of pushing message to the queue.
		void
		try_to_store_message_to_queue(
			const std::type_index & msg_type,
			const message_ref_t & message,
			invocation_type_t demand_type )
			{
				std::unique_lock< std::mutex > lock{ m_lock };

				// If queue full and waiting on full queue is enabled we
				// must wait for some time until there will be some space in
				// the queue.
				bool queue_full = m_queue.is_full();
				if( m_queue.is_full() && m_capacity.is_overflow_timeout_defined() )
					{
						m_overflow_cond.wait_for(
								lock,
								m_capacity.overflow_timeout(),
								[this, &queue_full] {
									return !(queue_full = m_queue.is_full());
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
										rc_msg_bag_overflow,
										"an attempt to push message to full msg_bag "
										"with overflow_reaction::throw_exception policy" );
							}
						else
							{
								so_5::details::abort_on_fatal_error( [&] {
//FIXME: implement this!
									} );
							}
					}

				// May be someone is waiting on empty queue?
				const bool queue_was_empty = m_queue.is_empty();
				
				m_queue.push_back(
						bag_demand_t{ msg_type, message, demand_type } );

				if( queue_was_empty )
					m_underflow_cond.notify_one();
			}
	};

} /* namespace msg_bag */

} /* namespace rt */

} /* namespace so_5 */

