/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Класс для обеспечения работы со слоями.
*/

#if !defined( _SO_5__RT__IMPL__LAYER_CORE_HPP_ )
#define _SO_5__RT__IMPL__LAYER_CORE_HPP_

#include <vector>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/so_layer.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

namespace impl
{

//
// typed_layer_ref_t
//

/*! Вспомогательный класс для хранения so_layer_ref_t
	вместе с актуальным типом слоя.
*/
struct typed_layer_ref_t
{
	typed_layer_ref_t();
	typed_layer_ref_t( const so_layer_map_t::value_type & v );
	typed_layer_ref_t(
		const type_wrapper_t & type,
		const so_layer_ref_t & layer );

	bool
	operator < ( const typed_layer_ref_t & tl ) const;

	//! Обертка на настоящий тип слоя.
	type_wrapper_t m_true_type;

	//! Слой.
	so_layer_ref_t m_layer;
};

//! Псевдоним для списка typed_layer_ref_t.
typedef std::vector< typed_layer_ref_t >
	so_layer_list_t;

//
// layer_core_t
//

//! Класс для обеспечения работы со слоями.
/*!
	Существует 2 сценария добавления слоя.

	Первый подразумевает определение слоев по умолчанию (изначально).
	Такие слои должны быть переданы в конструктор.
	Слои по умолчанию могут быть активынми \see so_layer_t.
	До старта Sobjectizer-а у слоев по умолчанию вызывается
	метод so_layer_t::start(), в котором можно задать некоторые действия
	для подлготовки слоя к работе. А при завершении работы
	Sobjectizer-а у таких слоев вызывается метод
	so_layer_t::shutdown(), для инициирования завршения работы слоя
	и so_layer_t::wait() для того чтобы убедиться, что слой
	завершил работу.

	Второй сценарий позволяет добавлять дополнительные
	слои в ходе работы Sobjectizer. Методы so_layer_t::start(),
	у таких слоев вызываются уже во время работы Sobjectizer.

	Деинициализация дополнительных слоев должна произайти до
	деинициализации основных слоев.

	\note Добавление слоя который в своем методе so_layer_t::start()
	полагается на то, что Sobjectizer еще не запущен
	может привести к ошибкам и аварийному завершению приложения.
*/
class layer_core_t
{
	public:
		layer_core_t(
			//! Слои.
			const so_layer_map_t & so_layers,
			//! Среда SObjectizer к которой привязывать
			//! накладываемые слои.
			so_environment_t * env );
		~layer_core_t();

		//! Получить слой.
		so_layer_t *
		query_layer(
			const type_wrapper_t & type ) const;

		//! Запустить работу слоев.
		ret_code_t
		start();

		//! Остановить работу дополнительных слоев слоев.
		void
		shutdown_extra_layers();

		//! Ждать завершения всех слоев.
		/*!
			После завершения работы слоев по умолчанию,
			удаляются все дополнительные слои.
		*/
		void
		wait_extra_layers();

		//! Остановить работу слоев.
		void
		shutdown_default_layers();

		//! Ждать завершения всех слоев.
		/*!
			После завершения работы слоев по умолчанию,
			удаляются все дополнительные слои.
		*/
		void
		wait_default_layers();

		//! Добавить дополнительный слой.
		/*!
			Если такого слоя нет среди
			слоев по умолчанию и среди дополнительный слоев,
			то он добавляется в список дополнительных слоев.

			Перед добавлением слой привязывается к so_environment
			и у него вызывается метод инициализации
			so_layer_t::start().
		*/
		ret_code_t
		add_extra_layer(
			const type_wrapper_t & type,
			const so_layer_ref_t & layer,
			throwing_strategy_t throwing_strategy );

	private:
		//! Среда SObjectizer, к которой привязывать
		//! накладываемые слои.
		so_environment_t * m_env;

		//! Список слоев по умолчанию.
		/*!
			Инициализируется в конструкторе и больше не меняется.
		*/
		so_layer_list_t m_default_layers;

		//! Замок для синхронизации
		mutable ACE_RW_Thread_Mutex m_extra_layers_lock;

		//! Список дополнительных слоев.
		/*!
			Слои могут добавляться и исключаться
			в ходе работы Sobjectizer.
		*/
		so_layer_list_t m_extra_layers;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
