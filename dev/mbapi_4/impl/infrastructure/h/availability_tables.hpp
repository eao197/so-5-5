/*
	MBAPI 4.
*/

/*!
	\file
	\brief Описания таблицы доступности конечных точек и стадий,
	а также таблица цепочек следования к конечной точке
	для тех, что доступны в данный момент.
*/

#if !defined( _MBAPI_4__IMPL__INFRASTRUCTURE__AVAILABILITY_TABLES_HPP_ )
#define _MBAPI_4__IMPL__INFRASTRUCTURE__AVAILABILITY_TABLES_HPP_

#include <map>

#include <so_5_transport/h/ifaces.hpp>

#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/defs/h/mbapi_node_uid.hpp>
#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>

namespace mbapi_4
{

namespace impl
{

namespace infrastructure
{

//
// available_endpoint_t
//

//! Запись о доступной конечной точке.
struct available_endpoint_t
{
	available_endpoint_t();

	//! Идентификатор канала из которого виден данная конечная точка.
	/*!
		Значение актуально только для таблицы самого узла.
		Для таблицы, которая приходит для синхронизации,
		содержимое m_channel_id не имеет никакого значения,
		потому как идентификаторы каналов другого SO процесса
		не имекет никакого значения для данного SO процесса.
	*/
	channel_uid_wrapper_t m_channel_id;

	//! Идентификатор mbapi-узла,
	//! в котором зарегистрирована данная конечная точка.
	mbapi_node_uid_t m_node_uid;

	//! Расстояние до mbapi-узла,
	//! в котором зарегистрирована данная конечная точка.
	/*!
		Расстояние узла до самого себя равно 0.
		Расстояние между узлами, которые связаны каналом, 1.
		Расстояние между узлами, которые не связаны между собой,
		но есть узел, который напрямую связан с ними, равно 2,
		и т.д.

		При синхронизации таблицы конечных точек с такой же таблицей
		соседнего узла надо учитывать, что расстояние каждый узел
		сообщает таким какое оно является для него.
	*/
	unsigned int m_distance;

	//! Описание стадий конечной точки.
	endpoint_stage_chain_t m_endpoint_stage_chain;
};

//! Тип умного указателя для available_endpoint_t.
typedef std::shared_ptr< available_endpoint_t >
	available_endpoint_ptr_t;

//! Тип таблицы с информацией о конечных точках.
typedef std::map<
		endpoint_t,
		available_endpoint_ptr_t >
	available_endpoint_table_t;

//
// available_stagepoint_t
//

//! Запись о стадии.
struct available_stagepoint_t
{
	available_stagepoint_t();

	//! Идентификатор канала из которого виден данная конечная точка.
	/*!
		Значение актуально только для таблицы самого узла.
		Для таблицы, которая приходит для синхронизации,
		содержимое m_channel_id не имеет никакого значения,
		потому как идентификаторы каналов другого SO процесса
		не имекет никакого значения для данного SO процесса.
	*/
	channel_uid_wrapper_t m_channel_id;

	//! Идентификатор mbapi-узла,
	//! в котором зарегистрирована данная конечная точка.
	mbapi_node_uid_t m_node_uid;

	//! Расстояние до mbapi-узла,
	//! в котором зарегистрирована данная конечная точка.
	/*!
		Расстояние узла до самого себя равно 0.
		Расстояние между узлами, которые связаны каналом, 1.
		Расстояние между узлами, которые не связаны между собой,
		но есть узел, который напрямую связан с ними, равно 2,
		и т.д.

		При синхронизации таблицы точек-стадий с такой же таблицей
		соседнего узла надо учитывать, что расстояние каждый узел
		сообщает таким какое оно является для него.
	*/
	size_t m_distance;
};

//! Тип умного указателя для available_stagepoint_t.
typedef std::shared_ptr< available_stagepoint_t >
	available_stagepoint_ptr_t;

//! Тип таблицы с информацией о стадиях.
typedef std::map<
		stagepoint_t,
		available_stagepoint_ptr_t >
	available_stagepoint_table_t;

} /* namespace infrastructure */

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
