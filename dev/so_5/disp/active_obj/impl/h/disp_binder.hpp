/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс привязки агента к диспетчеру active_obj.
*/

#if !defined( _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_BINDER_HPP_ )
#define _SO_5__DISP__ACTIVE_OBJ__IMPL__DISP_BINDER_HPP_

#include <so_5/rt/h/disp_binder.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/impl/h/so_environment_impl.hpp>

namespace so_5
{

namespace disp
{

namespace active_obj
{

namespace impl
{

//! Привязыватель агента к диспетчеру активных объектов.
class disp_binder_t
	:
		public so_5::rt::disp_binder_t
{
	public:
		explicit disp_binder_t(
			const std::string & disp_name );
		virtual ~disp_binder_t();

		//! Сделать привязку агента к диспетчеру.
		virtual void
		bind_agent(
			//! Среда so_5, к диспетчеру которой происходит привязка.
			so_5::rt::impl::so_environment_impl_t & env,
			//! Агент, который привязывается к диспетчеру.
			so_5::rt::agent_ref_t & agent_ref );

		//! Убрать привязку агента к диспетчеру.
		virtual void
		unbind_agent(
			//! Среда so_5, к диспетчеру которой привязан агент.
			so_5::rt::impl::so_environment_impl_t & env,
			//! Агент, который был привязан к диспетчеру.
			so_5::rt::agent_ref_t & agent_ref );

	private:
		//! Имя диспетчера к которому должен быть привязан агент.
		const std::string m_disp_name;
};

} /* namespace impl */

} /* namespace active_obj */

} /* namespace disp */

} /* namespace so_5 */

#endif
