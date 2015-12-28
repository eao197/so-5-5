/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A class for the agent state definition.
*/

#pragma once

#include <string>
#include <map>
#include <set>

#include <so_5/h/compiler_features.hpp>
#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/mbox_fwd.hpp>
#include <so_5/rt/h/fwd.hpp>

namespace so_5
{

#if defined( SO_5_MSVC )
	#pragma warning(push)
	#pragma warning(disable: 4251)
#endif

//FIXME: Document this!
//
// initial_substate_of
//
/*!
 * \since v.5.5.15
 */
struct initial_substate_of
{
	state_t * m_parent_state;

	initial_substate_of( state_t & parent_state )
		:	m_parent_state{ &parent_state }
		{}
};

//FIXME: Document this!
//
// substate_of
//
/*!
 * \since v.5.5.15
 */
struct substate_of
{
	state_t * m_parent_state;

	substate_of( state_t & parent_state )
		:	m_parent_state{ &parent_state }
		{}
};

//
// state_t
//

//! Class for the representing agent state.
class SO_5_TYPE state_t final
{
		state_t( const state_t & ) = delete;
		state_t & operator =( const state_t & ) = delete;

	public:
		/*!
		 * \brief Constructor without user specified name.
		 *
		 * A name for the state will be generated automatically.
		 */
		explicit state_t(
			agent_t * agent );
		/*!
		 * \brief Full constructor.
		 */
		state_t(
			agent_t * agent,
			std::string state_name );
		/*!
		 * \since v.5.5.15
		 * \brief Constructor for the case when state is the initial
		 * substate of some parent state.
		 */
		state_t(
			//! Parent state.
			initial_substate_of parent );
		/*!
		 * \since v.5.5.15
		 * \brief Constructor for the case when state is the initial
		 * substate of some parent state.
		 */
		state_t(
			//! Parent state.
			initial_substate_of parent,
			//! State name.
			std::string state_name );
		/*!
		 * \since v.5.5.15
		 * \brief Constructor for the case when state is a substate of some
		 * parent state.
		 */
		state_t(
			//! Parent state.
			substate_of parent );
		/*!
		 * \since v.5.5.15
		 * \brief Constructor for the case when state is a substate of some
		 * parent state.
		 */
		state_t(
			//! Parent state.
			substate_of parent,
			//! State name.
			std::string state_name );
		/*!
		 * \since v.5.4.0
		 * \brief Move constructor.
		 */
		state_t( state_t && other );

		~state_t();

		bool
		operator == ( const state_t & state ) const;

		//! Get textual name of the state.
		/*!
		 * \note The return type is changed in v.5.5.15: now it is a std::string
		 * object, not a const reference to some value inside state_t object.
		 */
		std::string
		query_name() const;

		//! Does agent owner of this state?
		bool
		is_target( const agent_t * agent ) const;

		/*!
		 * \since v.5.5.1
		 * \brief Switch agent to that state.
		 */
		void
		activate() const;

		/*!
		 * \since v.5.5.15
		 * \brief Get a parent state if exists.
		 */
		const state_t *
		parent_state() const
			{
				return m_parent_state;
			}

		/*!
		 * \since v.5.5.15
		 * \brief Find actual state to be activated for agent.
		 *
		 * \note If (*this) is a composite state then actual state to
		 * enter will be its m_initial_substate (if m_initial_substate is
		 * a composite state then actual state to enter will be its
		 * m_initial_substate and so on).
		 *
		 * \throw exception_t if (*this) is a composite state but m_initial_substate
		 * is not defined.
		 */
		const state_t *
		actual_state_to_enter() const;

		/*!
		 * \since v.5.5.1
		 * \brief Helper for subscription of event handler in this state.
		 *
		 * \note This method must be used for messages which are
		 * sent to agent's direct mbox.
		 *
		 * \par Usage example
			\code
			class my_agent : public so_5::agent_t
			{
				const so_5::state_t st_normal = so_make_state();
			public :
				...
				virtual void so_define_agent() override {
					st_normal.event( [=]( const msg_reconfig & evt ) { ... } );
					st_normal.event( &my_agent::evt_shutdown );
					...
				}
			};
			\endcode
		 */
		template< typename... ARGS >
		const state_t &
		event( ARGS&&... args ) const;

		/*!
		 * \since v.5.5.1
		 * \brief Helper for subscription of event handler in this state.
		 *
		 * \note This method must be used for messages which are
		 * sent to \a from message-box.
		 *
		 * \par Usage example
			\code
			class my_agent : public so_5::agent_t
			{
				const so_5::state_t st_normal = so_make_state();
			public :
				...
				virtual void so_define_agent() override {
					st_normal.event( m_owner, [=]( const msg_reconfig & evt ) { ... } );
					st_normal.event( m_owner, &my_agent::evt_shutdown );
					...
				}
			private :
				so_5::mbox_t m_owner;
			};
			\endcode
		 */
		template< typename... ARGS >
		const state_t &
		event( mbox_t from, ARGS&&... args ) const;

		/*!
		 * \since v.5.5.1
		 * \brief Helper for subscription of event handler in this state.
		 *
		 * \note This method must be used for signal subscriptions.
		 * \note This method must be used for messages which are
		 * sent to agent's direct mbox.
		 *
		 * \par Usage example
			\code
			class my_agent : public so_5::agent_t
			{
				const so_5::state_t st_normal = so_make_state();
			public :
				...
				virtual void so_define_agent() override {
					st_normal.event< msg_reconfig >( [=] { ... } );
					st_normal.event< msg_shutdown >( &my_agent::evt_shutdown );
					...
				}
			};
			\endcode
		 */
		template< typename SIGNAL, typename... ARGS >
		const state_t &
		event( ARGS&&... args ) const;

		/*!
		 * \since v.5.5.1
		 * \brief Helper for subscription of event handler in this state.
		 *
		 * \note This method must be used for signal subscriptions.
		 * \note This method must be used for messages which are
		 * sent to \a from message-box.
		 *
		 * \par Usage example
			\code
			class my_agent : public so_5::agent_t
			{
				const so_5::state_t st_normal = so_make_state();
			public :
				...
				virtual void so_define_agent() override {
					st_normal.event< msg_reconfig >( m_owner, [=] { ... } );
					st_normal.event< msg_shutdown >( m_owner, &my_agent::evt_shutdown );
					...
				}
			private :
				so_5::mbox_t m_owner;
			};
			\endcode
		 */
		template< typename SIGNAL, typename... ARGS >
		const state_t &
		event( mbox_t from, ARGS&&... args ) const;

	private:
		//! Fully initialized constructor.
		/*!
		 * \since v.5.5.15
		 */
		state_t(
			//! Owner of this state.
			agent_t * target_agent,
			//! Name for this state.
			std::string state_name,
			//! Parent state. nullptr means that there is no parent state.
			state_t * parent_state );

		//! Owner of this state.
		agent_t * const m_target_agent;

		//! State name.
		std::string m_state_name;

		/*!
		 * \since v.5.5.15
		 * \brief Parent state.
		 *
		 * \note Value nullptr means that state is a top-level state and
		 * has no parent state.
		 *
		 * \note This pointer is not const because some modification of
		 * parent state must be performed via that pointer.
		 */
		state_t * m_parent_state;

		/*!
		 * \since v.5.5.15
		 * \brief The initial substate.
		 *
		 * \note Value nullptr means that state has no initial substate.
		 * If m_substate_count == 0 it is normal. It means that state is
		 * not a composite state. But if m_substate_count != 0 the value
		 * nullptr means that state description is incorrect.
		 */
		const state_t * m_initial_substate;

		/*!
		 * \since v.5.5.15
		 * \brief Number of substates.
		 *
		 * \note Value 0 means that state is not composite state and has no
		 * any substates.
		 */
		size_t m_substate_count;

		inline const state_t *
		self_ptr() const { return this; }

		/*!
		 * \since v.5.5.1
		 * \brief A helper for handle-methods implementation.
		 */
		template< typename... ARGS >
		const state_t &
		subscribe_message_handler(
			const mbox_t & from,
			ARGS&&... args ) const;

		/*!
		 * \since v.5.5.1
		 * \brief A helper for handle-methods implementation.
		 */
		template< typename SIGNAL, typename... ARGS >
		const state_t &
		subscribe_signal_handler(
			const mbox_t & from,
			ARGS&&... args ) const;
};

#if defined( SO_5_MSVC )
	#pragma warning(pop)
#endif

namespace rt
{

/*!
 * \deprecated Will be removed in v.5.6.0. Use so_5::state_t instead.
 */
using state_t = so_5::state_t;

} /* namespace rt */

} /* namespace so_5 */

