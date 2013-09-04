/*
	SObjectizer 5
*/

/*!
	\file
	\brief Error codes.
*/

#if !defined( _SO_5__RET_CODE_HPP_ )
#define _SO_5__RET_CODE_HPP_

#include <so_5/h/declspec.hpp>

#include <iostream>

namespace so_5
{

//! Type for return codes of API functions and methods.
typedef int ret_code_t;

//
// Standard error codes.
//

//! so_environment launch failed.
const int rc_environment_error = 1;

//! \name Error codes for agent's methods.
//! \{

//! Trying to switch to unknown state.
const int rc_agent_unknown_state = 10;

//! Agent is already bound for SObjectizer environment.
const int rc_agent_is_already_bind_to_env = 11;

//! Agent is already bound to dispatcher.
const int rc_agent_is_already_bind_to_disp = 12;

//! Dispatcher with that name is not found.
const int rc_named_disp_not_found = 13;

//! Agent is not bound to a cooperation.
const int rc_agent_has_no_cooperation = 14;

//! It is impossible to make cast to that type.
const int rc_agent_incompatible_type_conversion = 15;

//! \}

//! \name Error codes for cooperations of agents.
//! \{

//! A zero pointer detected.
const int rc_zero_ptr_to_coop = 20;

//! Not unique name for cooperation.
const int rc_coop_with_specified_name_is_already_registered = 21;

//! Cooperation has null reference to an agent or dispatcher binding.
const int rc_coop_has_references_to_null_agents_or_binders = 22;

//! There is no registered cooperation with that name.
const int rc_coop_has_not_found_among_registered_coop = 23;

//! Cooperation couldn't be registered.
const int rc_coop_define_agent_failed = 24;

//! \}


//! \name Error codes for dispatchers.
//! \{

//! Unable to start dispatcher.
const int rc_disp_start_failed = 30;

//! Unable to create dispatcher.
const int rc_disp_create_failed = 31;

//! Required and actual types of binding dispatcher in not the same.
const int rc_disp_type_mismatch = 32;

//! \}

//! \name Error codes for event handlers and message interceptors registration
//! \{

//! A handler for that event/mbox/state is already registered.
const int rc_evt_handler_already_provided = 40;

//! A handler for that event/mbox/state is not registered and cannot be
//! deregistered.
const int rc_no_event_handler_provided = 41;

//! Unable to deregister that handler.
/*!
 * There is a handler for event/mbox/state but is is another handler.
 * Not the one specified for deregistration.
 */
const int rc_event_handler_match_error = 42;

//! Agent doesn't own that state.
const int rc_agent_is_not_the_state_owner = 43;

//! An interceptor for event/mbox/state is already registered.
const int rc_intercept_handler_already_provided = 44;

//! An interceptor for event/mbox/state is not registered and
//! cannot be deregistered.
const int rc_no_interception_handler_provided = 45;

//! Unable to deregister that interceptor.
/*!
 * There is an interceptor for event/mbox/state but is is another handler.
 * Not the one specified for deregistration.
 */
const int rc_interception_handler_match_error = 46;
//! \}

//! \name Error codes for mboxes.
//! \{

//! Not unique mbox name.
const int rc_mbox_duplicating_name = 80;

//! The name of mbox is unknown.
const int rc_mbox_unable_to_find_mbox = 81;
//! \}

//! \name Error codes for delayed or repeated events.
//! \{

//! Unable to schedule timer event.
const int rc_unable_to_schedule_timer_act = 90;
//! \}

//! \name Error codes for layers.
//! \{

//! The layer is not bound to SObjectizer environment.
const int rc_layer_not_binded_to_so_env = 100;

//! Unable to bind layer by null pointer to it.
const int rc_trying_to_add_nullptr_extra_layer = 101;

//! The layer is already bound to SObjectizer environment as a default layer.
const int rc_trying_to_add_extra_layer_that_already_exists_in_default_list = 102;

//! The layer is already bound to SObjectizer environment as an extra layer.
const int rc_trying_to_add_extra_layer_that_already_exists_in_extra_list = 103;

//! Layer initialization failed.
const int rc_unable_to_start_extra_layer = 104;

//! A layer with the type specified doesn't exist.
const int rc_layer_does_not_exist = 105;
//! \}

//! \name Common error codes.
//! \{

//! Empty name doesn't allowed.
const int rc_empty_name = 500;

//! Unclassified error.
const int rc_unexpected_error = 0xFFFFFF;
//! \}

} /* namespace so_5 */

#endif
