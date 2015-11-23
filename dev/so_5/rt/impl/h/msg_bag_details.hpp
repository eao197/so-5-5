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

namespace so_5 {

namespace rt {

namespace msg_bag {

namespace details {

//
// demand_type_t
//
/*!
 * \since v.5.5.13
 * \brief Type of demand in message bag.
 */
enum class demand_type_t
	{
		//! Ordinary asynchonous message.
		async_message,
		//! Service request.
		svc_request
	};

//
// bag_demand_t
//
/*!
 * \since v.5.5.13
 * \brief Description of one demand in message bag.
 */
struct bag_demand_t
	{
		//! Type of the message.
		std::type_index m_msg_type;
		//! Event incident.
		message_ref_t m_message_ref;
		//! Type of demand.
		demand_type_t m_demand_type;

// NOTE: the full set of constructors and copy/move operators is defined
// because VC++12.0 doesn't generate move constructors/operators automatically
// and doesn't support '=default' construct.

		//! Default constructor.
		bag_demand_t()
			:	m_msg_type( typeid(void) )
			{}
		//! Initializing constructor.
		bag_demand_t(
			std::type_index msg_type,
			message_ref_t message_ref,
			demand_type_t demand_type )
			:	m_msg_type{ std::move(msg_type) }
			,	m_message_ref{ std::move(message_ref) }
			,	m_demand_type{ demand_type }
			{}

		//! Copy constructor.
		bag_demand_t( const bag_demand_t & o )
			:	m_msg_type{ o.m_msg_type }
			,	m_message_ref{ o.m_message_ref }
			,	m_demand_type{ o.m_demand_type }
			{}
		//! Move constructor.
		bag_demand_t( bag_demand_t && o )
			:	m_msg_type{ std::move(o.m_msg_type) }
			,	m_message_ref{ std::move(o.m_message_ref) }
			,	m_demand_type{ std::move(o.m_demand_type) }
			{}

		//! Swap operation.
		void
		swap( bag_demand_t & o )
			{
				std::swap( m_msg_type, o.m_msg_type );
				m_message_ref.swap( o.m_message_ref );
				std::swap( m_demand_type, o.m_demand_type );
			}

		//! Copy operator.
		bag_demand_t &
		operator=( const bag_demand_t & o )
			{
				bag_demand_t tmp{ o };
				tmp.swap( *this );
				return *this;
			}

		//! Move operator.
		bag_demand_t &
		operator=( bag_demand_t && o )
			{
				bag_demand_t tmp{ std::move(o) };
				tmp.swap( *this );
				return *this;
			}
	};

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
			//! Maximun size of the queue.
			std::size_t max_size )
			:	m_max_size{ max_size }
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
			//! Maximun size of the queue.
			std::size_t max_size )
			:	m_storage( max_size, bag_demand_t{} )
			,	m_max_size{ max_size }
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

} /* namespace msg_bag */

} /* namespace rt */

} /* namespace so_5 */

