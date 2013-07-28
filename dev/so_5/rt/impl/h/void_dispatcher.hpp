/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Реализация интерфейс диспетчера - so_5::rt::dispatcher_t,
	которая ничего не делает. Объект данного класс подставляется всем
	агентам в качестве начального диспетчера, до того, как они
	будут привязаны к реальному диспетчеру.
*/

#if !defined( _SO_5__RT__IMPL__VOID_DISPATCHER_HPP_ )
#define _SO_5__RT__IMPL__VOID_DISPATCHER_HPP_

#include <so_5/rt/h/disp.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// void_dispatcher_t
//

//! Класс - заглушка для диспетчера, которая подставляется
//! агентам при их создании.
class void_dispatcher_t
	:
		public dispatcher_t
{
	public:
		void_dispatcher_t();
		virtual ~void_dispatcher_t();

		//! Реализация методов базового класса.
		//! \{
		virtual ret_code_t
		start();

		virtual void
		shutdown();

		virtual void
		wait();

		virtual void
		put_event_execution_request(
			//! Агент событие которого надо запланировать.
			const agent_ref_t & agent_ref,
			//! Количество событий,
			//! которые должны произайти у этого агента.
			unsigned int request_count );
		//! \}
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
