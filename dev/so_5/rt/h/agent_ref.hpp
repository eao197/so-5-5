/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Smart reference to the agent definition.
*/

#if !defined( _SO_5__RT__AGENT_REF_HPP_ )
#define _SO_5__RT__AGENT_REF_HPP_

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class agent_t;

//! Smart reference to an agent.
/*!
 * Uses agent's reference counting mechanism.
 */
class SO_5_TYPE agent_ref_t
{
	public:
		/*! Creates a null reference. */
		agent_ref_t();
		/*! Gets control on the \a agent. */
		explicit agent_ref_t(
			agent_t * agent );

		/*! Copy constructor. */
		agent_ref_t(
			const agent_ref_t & agent_ref );

		/*! Move constructor. */
		inline agent_ref_t(
			agent_ref_t && agent_ref )
			:	m_agent_ptr( agent_ref.release_pointer() )
		{}

		/*! Copy operator. */
		agent_ref_t &
		operator = ( const agent_ref_t & agent_ref );

		/*! Move operator. */
		agent_ref_t &
		operator = ( agent_ref_t && o );

		~agent_ref_t();

		//! Raw pointer to the refered agent.
		inline agent_t *
		get() const
		{
			return m_agent_ptr;
		}

		//! Access to the agent via pointer.
		inline agent_t *
		operator -> ()
		{
			return m_agent_ptr;
		}

		//! Access to the agent via pointer.
		inline const agent_t *
		operator -> () const
		{
			return m_agent_ptr;
		}

		//! Non-const reference to the agent.
		inline agent_t &
		operator * ()
		{
			return *m_agent_ptr;
		}

		//! Const reference to the agent.
		inline const agent_t &
		operator * () const
		{
			return *m_agent_ptr;
		}

		//! Is two references are refered to the same object?
		inline bool
		operator == ( const agent_ref_t & agent_ref ) const
		{
			return m_agent_ptr == agent_ref.m_agent_ptr;
		}

	private:
		//! Increment agent reference count.
		void
		inc_agent_ref_count();

		//! Decrement agent reference count.
		/*!
		 * Deletes agent if reference count became 0.
		 */
		void
		dec_agent_ref_count();

		/*!
		 * Release control on the agent.
		 */
		inline agent_t *
		release_pointer()
		{
			auto r = m_agent_ptr;
			m_agent_ptr = nullptr;
			return r;
		}

		//! Agent.
		agent_t * m_agent_ptr;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
