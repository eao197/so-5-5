/*
	MBAPI 4.
*/

/*!
	\file
	\brief Идентификатор узла mbapi сети.
*/

#if !defined( _MBAPI_4__DEFS__MBAPI_NODE_UID_HPP_ )
#define _MBAPI_4__DEFS__MBAPI_NODE_UID_HPP_

#include <string>

#include <mbapi_4/defs/h/declspec.hpp>

namespace mbapi_4
{

namespace comm
{

namespace impl
{

class iochannel_data_processor_t;

} /* namespace impl */

} /* namespace comm */

//
// mbapi_node_uid_t
//

//! Идентификатор узла mbapi сети.
/*!
	Инициализировать mbapi_node_uid_t необходимо только
	при получении информации из канала, когда идентификаторы узлов
	приходят в виде строки, поэтому
	класс mbapi_4::comm::impl::iochannel_data_processor_t
	является дружественным для mbapi_node_uid_t.
*/
class MBAPI_4_DEFS_TYPE mbapi_node_uid_t
{

		friend class mbapi_4::comm::impl::iochannel_data_processor_t;
		mbapi_node_uid_t(
			const std::string & uid );

	public:
		mbapi_node_uid_t();

		~mbapi_node_uid_t();

		//! Генерирует уникальный уникальный идентификатор.
		/*!
			Идентификатор генерируется с помощью
			ACE_Utils::UUID_GENERATOR.
		*/
		static mbapi_node_uid_t
		create();

		//! Генерирует уникальный уникальный идентификатор.
		static mbapi_node_uid_t
		utest_create_uid(
			const std::string & uid );

		//! Строковое представление идентификатора.
		inline const std::string &
		str() const
		{
			return m_uid;
		}

	private:
		//! Уникальный идентификатор узла.
		std::string m_uid;
};

} /* namespace mbapi_4 */

#endif
