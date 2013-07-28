/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Слой для работы с реакторами.
*/

#if !defined( _SO_5_TRANSPORT__REACTOR_LAYER_HPP_ )
#define _SO_5_TRANSPORT__REACTOR_LAYER_HPP_

#include <ace/Guard_T.h>
#include <ace/Synch.h>
#include <ace/Synch_Traits.h>
#include <ace/Thread_Mutex.h>
#include <ace/Hash_Map_Manager_T.h>

#include <so_5/rt/h/so_layer.hpp>
#include <so_5/rt/h/nonempty_name.hpp>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/ret_code.hpp>

#include <so_5_transport/ace/h/reactors.hpp>
#include <so_5_transport/ace/h/ace_hash_adaptation.hpp>

namespace so_5_transport
{

//
// reactor_instance_hash_map_t
//

//! Тип Hash_Map-а для хранения reactor_instance.
typedef ACE_Hash_Map_Manager_Ex<
		std::string,
		so_5_transport::ace::reactor_instance_ref_t,
		std_string_djb_hash_t,
		ACE_Equal_To< std::string >,
		ACE_Null_Mutex >
	reactor_instance_hash_map_t;

//
// reactor_layer_params_t
//

//! Класс для добавления параметров слоя реакторов
class SO_5_TRANSPORT_TYPE reactor_layer_params_t
{
	public:
		//! Добавить именованный реактор.
		/*!
			Если реактор с таким именем уже был добавлен,
			то он будет замещен текушим указанным.

			Если реактор reactor = 0, то добавление игнорируется.
		*/
		reactor_layer_params_t &
		add_named_reactor(
			//! Имя реактора.
			const so_5::rt::nonempty_name_t & name,
			//! Реактор.
			so_5_transport::ace::reactor_instance_unique_ptr_t reactor );

		const so_5_transport::ace::reactor_instance_map_t &
		query_named_reactor_map() const;

	private:
		so_5_transport::ace::reactor_instance_map_t
			m_named_reactor_map;
};

//
// reactor_layer_t
//

//! Класс для обеспечения работы с диспетчерами.
class SO_5_TRANSPORT_TYPE reactor_layer_t
	:
		public so_5::rt::so_layer_t
{
	public:
		reactor_layer_t();
		reactor_layer_t(
			const reactor_layer_params_t & params );
		virtual ~reactor_layer_t();

		//! Получить реактор по умолчанию.
		ACE_Reactor *
		query_default_reactor() const;

		//! Получить именованный реактор.
		/*!
			\return Если реактор с таким именем есть,
			то он и вернется, в противном случае вернется 0.
		*/
		ACE_Reactor *
		query_named_reactor(
			const std::string & reactor_name ) const;

	protected:
		//! \name Реализация унаследованных методов.
		//! \{
		//! Запустить работу реакторов.
		so_5::ret_code_t
		start();

		//! Остановить работу реакторов.
		void
		shutdown();

		//! Ждать завершения всех реакторов.
		void
		wait();
		//! \}

	private:
		//! Создать реактор по умолчанию.
		void
		create_default_reactor();

		//! Сохранить именованные реакторы.
		void
		create_named_reactors(
			const so_5_transport::ace::reactor_instance_map_t &
				named_reactors );

		//! Реактор по умолчанию.
		so_5_transport::ace::reactor_instance_unique_ptr_t m_default_reactor;

		//! Карта всех зарегистрированных реакторов.
		std::unique_ptr< reactor_instance_hash_map_t > m_reactors_map;
};

} /* namespace so_5_transport */

#endif
