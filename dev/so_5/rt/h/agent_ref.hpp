/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс ссылка на агента.
*/

#if !defined( _SO_5__RT__AGENT_REF_HPP_ )
#define _SO_5__RT__AGENT_REF_HPP_

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class agent_t;

//! Класс умная ссылка на agent_t.
class SO_5_TYPE agent_ref_t
{
	public:
		agent_ref_t();
		explicit agent_ref_t(
			agent_t * agent );

		agent_ref_t(
			const agent_ref_t & agent_ref );

		agent_ref_t &
		operator = ( const agent_ref_t & agent_ref );

		~agent_ref_t();

		inline agent_t *
		get() const
		{
			return m_agent_ptr;
		}

		inline agent_t *
		operator -> ()
		{
			return m_agent_ptr;
		}

		inline const agent_t *
		operator -> () const
		{
			return m_agent_ptr;
		}

		inline agent_t &
		operator * ()
		{
			return *m_agent_ptr;
		}

		inline const agent_t &
		operator * () const
		{
			return *m_agent_ptr;
		}

		inline bool
		operator == ( const agent_ref_t & agent_ref ) const
		{
			return m_agent_ptr == agent_ref.m_agent_ptr;
		}

	private:
		//! Увеличить количество ссылок на агента
		//! и в случае необходимости удалить его.
		void
		inc_agent_ref_count();

		//! Уменьшить количество ссылок на агента
		//! и в случае необходимости удалить его.
		void
		dec_agent_ref_count();

		//! Агент.
		agent_t * m_agent_ptr;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
