/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Public part of message bag related stuff.
 */

#pragma once

#include <so_5/rt/h/mbox.hpp>

#include <chrono>

namespace so_5 {

namespace rt {

namespace msg_bag {

class extraction_sink_t;
class bag_iface_t;

using clock = std::chrono::high_resolution_clock;

//
// storage_memory_t
//
/*!
 * \since v.5.5.13
 * \brief Memory allocation for storage for size-limited bags.
 */
enum class storage_memory_t
	{
		//! Storage can be allocated and deallocated dynamically.
		dynamic,
		//! Storage must be preallocated once and doesn't change after that.
		preallocated
	};

//
// overflow_reaction_t
//
/*!
 * \since v.5.5.13
 * \brief What reaction must be performed on attempt to push new message to
 * the full message bag.
 */
enum class overflow_reaction_t
	{
		//! Application must be aborted.
		abort_app,
		//! An exception must be thrown.
		throw_exception,
		//! New message must be ignored and droped.
		drop_newest,
		//! Oldest message in bag must be removed.
		remove_oldest
	};

//
// capacity_t
//
/*!
 * \since v.5.5.13
 * \brief Parameters for defining bag size.
 */
class capacity_t
	{
		//! Has bag unlimited size?
		bool m_unlimited = { true };

		// NOTE: all other atributes have sence only if m_unlimited != true.

		//! Max size of the bag with limited size.
		std::size_t m_max_size;

		//! Type of the storage for size-limited bag.
		storage_memory_t m_memory;

		//! Type of reaction for bag overflow.
		overflow_reaction_t m_overflow_reaction;

		//! Timeout for waiting on full bag during 'message push' operation.
		/*!
		 * \note Value 'zero' means that there must not be waiting on
		 * full bag.
		 */
		clock::duration m_overflow_timeout;

		//! Initializing constructor for size-limited message bag.
		capacity_t(
			std::size_t max_size,
			storage_memory_t memory,
			overflow_reaction_t overflow_reaction,
			clock::duration overflow_timeout )
			:	m_unlimited{ false }
			,	m_memory{ memory }
			,	m_overflow_reaction{ overflow_reaction }
			,	m_overflow_timeout( overflow_timeout )
			{}

	public :
		//! Default constructor.
		/*!
		 * Creates description for size-unlimited bag.
		 */
		capacity_t()
			{}

		//! Create capacity description for size-unlimited message bag.
		inline static capacity_t
		make_unlimited() { return capacity_t{}; }

		//! Create capacity description for size-limited message bag
		//! without waiting on full queue during 'push message' operation.
		inline static capacity_t
		make_limited_without_waiting(
			//! Max size of the bag.
			std::size_t max_size,
			//! Type of bag storage.
			storage_memory_t memory,
			//! Reaction on bag overflow.
			overflow_reaction_t overflow_reaction )
			{
				capacity_t c{
						max_size,
						memory,
						overflow_reaction,
						clock::duration::zero()
				};
			}

		//! Create capacity description for size-limited message bag
		//! with waiting on full queue during 'push message' operation.
		inline static capacity_t
		make_limited_with_waiting(
			//! Max size of the bag.
			std::size_t max_size,
			//! Type of bag storage.
			storage_memory_t memory,
			//! Reaction on bag overflow.
			overflow_reaction_t overflow_reaction,
			//! Waiting time on full message bag.
			clock::duration wait_timeout )
			{
				return capacity_t{
						max_size,
						memory,
						overflow_reaction,
						wait_timeout
				};
			}

		//! Is message bag have no size limit?
		bool
		unlimited() const { return m_unlimited; }

		//! Max size for size-limited bag.
		/*!
		 * \attention Has sence only for size-limited bags.
		 */
		std::size_t
		max_size() const { return m_max_size; }

		//! Memory allocation type for size-limited bag.
		/*!
		 * \attention Has sence only for size-limited bags.
		 */
		storage_memory_t
		memory() const { return m_memory; }

		//! Overflow reaction for size-limited bag.
		/*!
		 * \attention Has sence only for size-limited bags.
		 */
		overflow_reaction_t
		overflow_reaction() const { return m_overflow_reaction; }

		//! Is waiting timeout for overflow case defined?
		/*!
		 * \attention Has sence only for size-limited bags.
		 */
		bool
		is_overflow_timeout_defined() const
			{
				return clock::duration::zero() != m_overflow_timeout;
			}

		//! Get the value of waiting timeout for overflow case.
		/*!
		 * \attention Has sence only for size-limited bags.
		 */
		clock::duration
		overflow_timeout() const
			{
				return m_overflow_timeout;
			}
	};

} /* namespace msg_bag */

//
// abstract_message_bag_t
//
/*!
 * \since v.5.5.13
 * \brief An interace of message bag.
 */
class SO_5_TYPE abstract_message_bag_t : protected abstract_message_box_t
	{
		friend class intrusive_ptr_t< abstract_message_bag_t >;
		friend class msg_bag::bag_iface_t;

		abstract_message_bag_t( const abstract_message_bag_t & ) = delete;
		abstract_message_bag_t &
		operator=( const abstract_message_bag_t & ) = delete;

	protected :
		virtual ~abstract_message_bag_t();

		/*!
		 * \return count of messages extracted.
		 */
		virtual std::size_t
		extract_messages(
			//! Destination for extracted messages.
			msg_bag::extraction_sink_t & dest,
			//! Max count of messages to be extracted.
			std::size_t max_messages_to_extract,
			//! Max time to wait on empty queue.
			msg_bag::clock::duration empty_queue_timeout ) = 0;

	public :
		//! Cast message bag to message box.
		mbox_t
		as_mbox();

		//! Is message bag empty?
		virtual bool
		empty() const = 0;

		//! Count of messages in the bag.
		virtual std::size_t
		size() const = 0;
	};

//
// msg_bag_t
//
/*!
 * \since v.5.5.13
 * \brief Short name for smart pointer to message bag.
 */
using msg_bag_t = intrusive_ptr_t< abstract_message_bag_t >;

} /* namespace rt */

} /* namespace so_5 */

