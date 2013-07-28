/*
	MBAPI 4.
*/

/*!
	\file
	\brief ќписани€ таблицы доступности конечных точек и стадий,
	а также таблица цепочек следовани€ к конечной точке
	дл€ тех, что доступны в данный момент.
*/

#if !defined( _MBAPI_4__IMPL__INFRASTRUCTURE__CHANNEL_INFO_HPP_ )
#define _MBAPI_4__IMPL__INFRASTRUCTURE__CHANNEL_INFO_HPP_

#include <map>

#include <so_5/rt/h/mbox_ref.hpp>
#include <so_5/rt/h/mbox.hpp>

#include <so_5_transport/h/ifaces.hpp>

#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/defs/h/stagepoint.hpp>
#include <mbapi_4/defs/h/channel_uid_wrapper.hpp>
#include <mbapi_4/defs/h/mbapi_node_uid.hpp>

#include <mbapi_4/impl/infrastructure/h/availability_tables.hpp>

namespace mbapi_4
{

namespace impl
{

namespace infrastructure
{

//!  онтейнер дл€ хранени€ списка точек-стадий.
typedef std::vector< stagepoint_t > stagepoint_list_t;

//
// channel_info_t
//

//! »нформаци€ о канале.
/*!
	—одержит:
		\li uid mbapi-узла, который находитс€ с другой стороны канала;
		\li —писок конечных точек доступных из этого канала,
			которые хранит данный узел в своей талице конечных точек;
		\li —писок точек-стадий доступных из этого канала,
			которые хранит данный узел в своей талице стадий.
*/
class channel_info_t
{
	public:
		channel_info_t();
		channel_info_t(
			const mbapi_node_uid_t & mbapi_node_uid,
			const so_5::rt::mbox_ref_t & mbox );

		//! »дентификатор mbapi-узла.
		inline const mbapi_node_uid_t &
		node_uid() const
		{
			return m_node_uid;
		}

		//! ѕолучить mbox канала.
		inline const so_5::rt::mbox_ref_t &
		mbox() const
		{
			return m_mbox;
		}

		//! ѕолучить список конечных точек, которых
		//! нет в заданной таблице конечных точек.
		/*!
			»счезнувшие конечные точки удал€ютс€ из списка.
		*/
		void
		find_dissapered_endpoints(
			//! “аблица конечных точек.
			const available_endpoint_table_t & endpoint_table,
			//! ѕриемник списка исчезнувших конечных точек.
			endpoint_list_t & dissapered_endpoints );

		//! ѕолучить список точек-стадий, которых
		//! нет в заданной таблице точек-стадий.
		/*!
			»счезнувшие точки-стадии удал€ютс€ из списка.
		*/
		void
		find_dissapered_stagepoints(
			//! “аблица точек-стадий.
			const available_stagepoint_table_t & stagepoint_table,
			//! ѕриемник списка исчезнувших точек-стадий.
			stagepoint_list_t & dissapered_stagepoints );

		//! ƒабавление и исключение конечных точек и стадий.
		//! \{
		void
		insert_endpoint(
			const endpoint_t & endpoint );

		void
		remove_endpoint(
			const endpoint_t & endpoint );

		void
		insert_stagepoint(
			const stagepoint_t & stagepoint );

		void
		remove_stagepoint(
			const stagepoint_t & stagepoint );
		//! \}

		//! ѕолучить список точек.
		//! \{
		inline const endpoint_list_t &
		endpoints() const
		{
			return m_endpoints;
		}


		inline const stagepoint_list_t &
		stagepoints() const
		{
			return m_stagepoints;
		}
		//! \}

	private:
		//! »дентификатор mbapi-узла с которым установлен канал,
		mbapi_node_uid_t m_node_uid;

		//! Mbox канала.
		so_5::rt::mbox_ref_t m_mbox;

		//! —писиок конечных точек.
		endpoint_list_t m_endpoints;

		//! —писиок точек-стадий.
		stagepoint_list_t m_stagepoints;
};

//! “ип умного указател€ дл€ channel_info_t.
typedef std::shared_ptr< channel_info_t >
	channel_info_ptr_t;

//! “аблица с информацией о каналах.
typedef std::map<
		channel_uid_wrapper_t,
		channel_info_ptr_t >
	channel_info_table_t;


} /* namespace infrastructure */

} /* namespace impl */

} /* namespace mbapi_4 */

#endif
