/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Public part of message bag related stuff.
 */

#include <so_5/rt/h/msg_bag.hpp>

namespace so_5 {

namespace rt {

namespace msg_bag {

} /* namespace msg_bag */

//
// abstract_message_bag_t
//
abstract_message_bag_t::abstract_message_bag_t()
	{}
abstract_message_bag_t::~abstract_message_bag_t()
	{}

mbox_t
abstract_message_bag_t::as_mbox()
	{
		return mbox_t{ this };
	}

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

