/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.4
 * \file
 * \brief Helpers for do rollback actions in the case of exception.
 */

#pragma once

namespace so_5 {

namespace details {

namespace rollback_on_exception_details {

/*!
 * \since v.5.5.4
 * \brief Helper template class for do rollback actions automatically
 * in the destructor.
 *
 * \tparam L type of lambda with rollback actions.
 */
template< typename L >
class rollbacker_t
	{
		L & m_action;
		bool m_commited = false;

	public :
		inline rollbacker_t( L & action ) : m_action{ action } {}
		inline ~rollbacker_t() { if( !m_commited ) m_action(); }

		inline void commit() { m_commited = true; }
	};

} /* namespace rollback_on_exception_details */

/*!
 * \since v.5.5.4
 * \brief Helper function for do some action with rollback in the case of
 * an exception.
 *
 * \tparam MAIN_ACTION type of lambda with main action.
 * \tparam ROLLBACK_ACTION type of lambda with rollback action.
 */
template< typename MAIN_ACTION, typename ROLLBACK_ACTION >
void
do_with_rollback_on_exception(
	MAIN_ACTION main_action,
	ROLLBACK_ACTION rollback_action )
	{
		using namespace rollback_on_exception_details;

		rollbacker_t< ROLLBACK_ACTION > rollbacker{ rollback_action };

		main_action();
		rollbacker.commit();
	}

} /* namespace details */

} /* namespace so_5 */

