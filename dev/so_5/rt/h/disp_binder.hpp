/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Интерфейс привязки агента к диспетчеру,
	и его стандартная реализация.
*/

#if !defined( _SO_5__RT__DISP_BINDER_HPP_ )
#define _SO_5__RT__DISP_BINDER_HPP_

#include <memory>

#include <so_5/h/declspec.hpp>
#include <so_5/rt/h/agent_ref.hpp>
#include <so_5/rt/h/disp.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

class so_environment_impl_t;

} /* namespace impl */


class so_environment_t;

//
// disp_binder_t
//

//! Класс - интерфейс привязки агента к диспетчеру.
/*!
	Когда кооперация, в которую входит агент, регистрируется
	агентов надо привязывать к диспетчеру, который будет отвечать
	за выполнение событий агента.

	По умолчанию кооперации назначается привязываитель,
	который связывает агентов с диспетчером пассивных
	агентов (по умолчанию - 1 нить на всех). Но, если агента надо привязать
	к именованному диспетчеру, привязка к которому требует дополнительных
	действий, то для такого диспетчера реализуется интерфейс привязывателя,
	который переопределяет метод привязки и избавления от привязки.
*/
class SO_5_TYPE disp_binder_t
{
	public:
		disp_binder_t();
		virtual ~disp_binder_t();

		//! Сделать привязку агента к диспетчеру.
		virtual void
		bind_agent(
			//! Среда SO к которой принадлежит агент,
			//! его кооперация и к диспетчеру которой происходит привязка.
			impl::so_environment_impl_t & env,
			//! Агент которого надо привязать к диспетчеру.
			agent_ref_t & agent_ref ) = 0;

		//! Избавиться от привязки агента к диспетчеру.
		virtual void
		unbind_agent(
			//! Среда SO к которой принадлежит агент,
			//! его кооперация и к диспетчеру которой агент привязан.
			impl::so_environment_impl_t & env,
			//! Агент, который привязан к диспетчеру.
			agent_ref_t & agent_ref ) = 0;
};

//! Тип для умного указателя на disp_binder_t.
typedef std::unique_ptr< disp_binder_t > disp_binder_unique_ptr_t;

//! Тип для умного указателя на dispatcher_t.
typedef std::shared_ptr< disp_binder_t >
	disp_binder_ref_t;

//! Получить привязыватель по умолчанию.
SO_5_EXPORT_FUNC_SPEC( disp_binder_unique_ptr_t )
create_default_disp_binder();

} /* namespace rt */

} /* namespace so_5 */

#endif
