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

