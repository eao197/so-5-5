/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Public part of message chain related stuff.
 */

#include <so_5/rt/h/mchain.hpp>

namespace so_5 {

//
// abstract_message_chain
//
abstract_message_chain::abstract_message_chain()
	{}
abstract_message_chain::~abstract_message_chain()
	{}

so_5::rt::mbox_t
abstract_message_chain::as_mbox()
	{
		return so_5::rt::mbox_t{ this };
	}

} /* namespace so_5 */

