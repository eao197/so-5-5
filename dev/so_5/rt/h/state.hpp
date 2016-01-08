/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A class for the agent state definition.
*/

#pragma once

#include <array>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <chrono>

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
/*!
 * \attention This class is not thread safe. It is designed to be used inside
 * an owner agent only. For example:
 * \code
	class my_agent : public so_5::agent_t
	{
		state_t first_state{ this, "first" };
		state_t second_state{ this, "second" };
	...
	public :
		my_agent( context_t ctx ) : so_5::agent_t{ ctx }
		{
			// It is a safe usage of state.
			first_state.on_enter( &my_agent::first_on_enter );
			second_state.on_exit( &my_agent::second_on_exit );
			...
		}

		virtual void so_define_agent() override
		{
			// It is a safe usage of state.
			first_state.event( &my_agent::some_event_handler );
			second_state.time_limit( std::chrono::seconds{20}, first_state );

			second_state.event( [this]( const some_message & msg ) {
					// It is also safe usage of state because event handler
					// will be called on the context of agent's working thread.
					second_state.drop_time_limit();
					...
				} );
		}

		void some_public_method()
		{
			// It is a safe usage if this method is called by the agent itself.
			// And unsafe usafe otherwise.
			// If this method is called by someone else a data damage or
			// something like that can happen.
			second_state.time_limit( std::chrono::seconds{30}, first_state );
		}
	...
	};
 * \endcode
 * Because of that be very careful during manipulation of agent's states outside
 * of agent's event handlers.
 */
class SO_5_TYPE state_t final
{
		struct time_limit_t;

		friend class agent_t;

		state_t( const state_t & ) = delete;
		state_t & operator =( const state_t & ) = delete;

	public:
		/*!
		 * \since v.5.5.15
		 * \brief Max deep of nested states.
		 */
		static const std::size_t max_deep = 16;

		/*!
		 * \since v.5.5.15
		 * \brief Type of history for state.
		 */
		enum class history_t
		{
			//! State has no history.
			none,
			//! State has shallow history.
			shallow,
			//! State has deep history.
			deep
		};

		/*!
		 * \since v.5.5.15
		 * \brief Type of function to be called on enter to the state.
		 */
		using on_enter_handler_t = std::function< void() >;

		/*!
		 * \since v.5.5.15
		 * \brief Type of function to be called on exit from the state.
		 */
		using on_exit_handler_t = std::function< void() >;

		/*!
		 * \since v.5.5.15
		 * \brief Alias for duration type.
		 */
		using duration_t = std::chrono::high_resolution_clock::duration;

		/*!
		 * \note State name will be generated automaticaly.
		 */
		state_t(
			//! State owner.
			agent_t * agent );
		/*!
		 * \since v.5.5.15
		 * \note State name will be generated automaticaly.
		 */
		state_t(
			//! State owner.
			agent_t * agent,
			//! Type of state history.
			history_t state_history );
		state_t(
			//! State owner.
			agent_t * agent,
			//! State name.
			std::string state_name );
		/*!
		 * \since v.5.5.15
		 */
		state_t(
			//! State owner.
			agent_t * agent,
			//! State name.
			std::string state_name,
			//! Type of state history.
			history_t state_history );
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
			//! Type of state history.
			history_t state_history );
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
		 * \brief Constructor for the case when state is the initial
		 * substate of some parent state.
		 */
		state_t(
			//! Parent state.
			initial_substate_of parent,
			//! State name.
			std::string state_name,
			//! Type of state history.
			history_t state_history );
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
			//! Type of state history.
			history_t state_history );
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
		 * \since v.5.5.15
		 * \brief Constructor for the case when state is a substate of some
		 * parent state.
		 */
		state_t(
			//! Parent state.
			substate_of parent,
			//! State name.
			std::string state_name,
			//! Type of state history.
			history_t state_history );
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
		 * \since v.5.5.15
		 */
		template< typename... ARGS >
		state_t &
		event( ARGS&&... args )
			{
				const state_t & t = *this;
				t.event( std::forward<ARGS>(args)... );
				return *this;
			}

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
		 * \since v.5.5.15
		 */
		template< typename... ARGS >
		state_t &
		event( mbox_t from, ARGS&&... args )
			{
				const state_t & t = *this;
				t.event( std::move(from), std::forward<ARGS>(args)... );
				return *this;
			}

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
		 * \since v.5.5.15
		 */
		template< typename SIGNAL, typename... ARGS >
		state_t &
		event( ARGS&&... args )
			{
				const state_t & t = *this;
				t.event< SIGNAL >( std::forward<ARGS>(args)... );
				return *this;
			}

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

		/*!
		 * \since v.5.5.15
		 */
		template< typename SIGNAL, typename... ARGS >
		state_t &
		event( mbox_t from, ARGS&&... args )
			{
				const state_t & t = *this;
				t.event< SIGNAL >( std::move(from), std::forward<ARGS>(args)... );
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		transfer_to_state( mbox_t from, const state_t & target_state ) const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		transfer_to_state( const state_t & target_state ) const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		transfer_to_state( mbox_t from, const state_t & target_state )
			{
				const state_t & t = *this;
				t.transfer_to_state< MSG >( std::move(from), target_state );
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		transfer_to_state( const state_t & target_state )
			{
				const state_t & t = *this;
				t.transfer_to_state< MSG >( target_state );
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		just_switch_to( mbox_t from, const state_t & target_state ) const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		just_switch_to( const state_t & target_state ) const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		just_switch_to( mbox_t from, const state_t & target_state )
			{
				const state_t & t = *this;
				t.just_switch_to< MSG >( std::move(from), target_state );
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		just_switch_to( const state_t & target_state )
			{
				const state_t & t = *this;
				t.just_switch_to< MSG >( target_state );
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		suppress() const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		const state_t &
		suppress( mbox_t from ) const;

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		suppress()
			{
				const state_t & t = *this;
				t.suppress< MSG >();
				return *this;
			}

		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 */
		template< typename MSG >
		state_t &
		suppress( mbox_t from )
			{
				const state_t & t = *this;
				t.suppress< MSG >( std::move(from) );
				return *this;
			}

		/*!
		 * \name Method for manupulation of enter/exit handlers.
		 * \{
		 */
		/*!
		 * \since v.5.5.15
		 * \brief Set on enter handler.
		 */
		state_t &
		on_enter( on_enter_handler_t handler )
			{
				m_on_enter = std::move(handler);
				return *this;
			}

		/*!
		 * \since v.5.5.15
		 * \brief Set on enter handler.
		 */
		template< typename AGENT >
		state_t &
		on_enter( void (AGENT::*pfn)() );

		/*!
		 * \since v.5.5.15
		 * \brief Query on enter handler.
		 */
		const on_enter_handler_t &
		on_enter() const
			{
				return m_on_enter;
			}

		/*!
		 * \since v.5.5.15
		 * \brief Set on exit handler.
		 */
		state_t &
		on_exit( on_exit_handler_t handler )
			{
				m_on_exit = std::move(handler);
				return *this;
			}

		/*!
		 * \since v.5.5.15
		 * \brief Set on exit handler.
		 */
		template< typename AGENT >
		state_t &
		on_exit( void (AGENT::*pfn)() );

		/*!
		 * \since v.5.5.15
		 * \brief Query on enter handler.
		 */
		const on_exit_handler_t &
		on_exit() const
			{
				return m_on_exit;
			}
		/*!
		 * \}
		 */

		/*!
		 * \name Methods for dealing with state's time limit.
		 * \{
		 */
		//FIXME: write Doxygen comment!
		/*!
		 * \since v.5.5.15
		 * \brief Set up a time limit for the state.
		 */
		state_t &
		time_limit(
			//! Max duration of time for staying in this state.
			duration_t timeout,
			//! A new state to be switched to.
			const state_t & state_to_switch );

		/*!
		 * \since v.5.5.15
		 * \brief Drop time limit for the state if defined.
		 *
		 * \note Do nothing if a time limit is not defined.
		 */
		state_t &
		drop_time_limit();
		/*!
		 * \}
		 */

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
			state_t * parent_state,
			//! Nesting deep for this state. Value 0 means this state is
			//! a top-level state.
			std::size_t nested_level,
			//! Type of state history.
			history_t state_history );

		//! Owner of this state.
		agent_t * const m_target_agent;

		//! State name.
		/*!
		 * \note Since v.5.5.15 has empty value for anonymous state.
		 */
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
		 * \brief Type of state history.
		 */
		history_t m_state_history;

		/*!
		 * \since v.5.5.15
		 * \brief Last active substate.
		 *
		 * \note This attribute is used only if
		 * m_state_history != history_t::none. It holds a pointer to last
		 * active substate of this composite state.
		 */
		const state_t * m_last_active_substate;

		/*!
		 * \since v.5.5.15
		 * \brief Nesting level for state.
		 *
		 * \note Value 0 means that state is a top-level state.
		 */
		std::size_t m_nested_level;

		/*!
		 * \since v.5.5.15
		 * \brief Number of substates.
		 *
		 * \note Value 0 means that state is not composite state and has no
		 * any substates.
		 */
		size_t m_substate_count;

		/*!
		 * \since v.5.5.15
		 * \brief Handler for the enter to the state.
		 */
		on_enter_handler_t m_on_enter;

		/*!
		 * \since v.5.5.15
		 * \brief Handler for the exit from the state.
		 */
		on_exit_handler_t m_on_exit;

		/*!
		 * \since v.5.5.15
		 * \brief A definition of time limit for the state.
		 *
		 * \note Value nullptr means that time limit is not set.
		 */
		std::unique_ptr< time_limit_t > m_time_limit;

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

		/*!
		 * \name Methods to be used by agents.
		 * \{
		 */
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
		 * \note If this state is a composite state with history then
		 * m_last_active_substate value will be used (if it is not nullptr).
		 *
		 * \throw exception_t if (*this) is a composite state but m_initial_substate
		 * is not defined.
		 */
		const state_t *
		actual_state_to_enter() const;

		/*!
		 * \since v.5.5.15
		 * \brief Query nested level for the state.
		 */
		std::size_t
		nested_level() const
			{
				return m_nested_level;
			}

		/*!
		 * \since v.5.5.15
		 * \brief A helper method for building a path from top-level
		 * state to this state.
		 */
		void
		fill_path( std::array< const state_t *, max_deep > & path ) const
			{
				path[ m_nested_level ] = this;
				if( m_parent_state )
					m_parent_state->fill_path( path );
			}

		/*!
		 * \since v.5.5.15
		 * \brief A helper method which is used during state change for
		 * update state with history.
		 */
		void
		update_history_in_parent_states() const;

		/*!
		 * \since v.5.5.15
		 * \brief A special handler of time limit to be used on entering into state.
		 * \attention This method must be called only if m_time_limit is not null.
		 */
		void
		handle_time_limit_on_enter() const;

		/*!
		 * \since v.5.5.15
		 * \brief A special handler of time limit to be used on exiting from state.
		 * \attention This method must be called only if m_time_limit is not null.
		 */
		void
		handle_time_limit_on_exit() const;

		/*!
		 * \since v.5.5.15
		 * \brief Call for on enter handler if defined.
		 */
		void
		call_on_enter() const
			{
				if( m_on_enter ) m_on_enter();
				if( m_time_limit ) handle_time_limit_on_enter();
			}

		/*!
		 * \since v.5.5.15
		 * \brief Call for on exit handler if defined.
		 */
		void
		call_on_exit() const
			{
				if( m_time_limit ) handle_time_limit_on_exit();
				if( m_on_exit ) m_on_exit();
			}
		/*!
		 * \}
		 */
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

