/*
	MBAPI 4.
*/

/*!
	\file
	\brief Привязка конечной точки на данном узле.
*/

#if !defined( _MBAPI_4__ENDPOINT_BIND_HPP_ )
#define _MBAPI_4__ENDPOINT_BIND_HPP_

#include <so_5/rt/h/agent.hpp>

#include <mbapi_4/defs/h/endpoint.hpp>
#include <mbapi_4/h/message.hpp>

#include <mbapi_4/h/declspec.hpp>
#include <mbapi_4/h/stagepoint_bind.hpp>

namespace mbapi_4
{

//
// endpoint_bind_t
//

//! Привязка конечной точки.
/*!
	Представляет собой образ конечной точки на данном узле.
	С его помощью можно подписываться на сообщения
	адресуемые данной конечной точке и отправлять сообщения
	другим конечным точкам.

	Привязки конечных точек выдаются mbapi_layer_t,
	который контролирует их уникальность.

	При уничтожении привязки конечная точка
	автоматически перестает существовать на данном узле.
*/
class MBAPI_4_TYPE endpoint_bind_t
	:
		public stagepoint_bind_t
{
	friend class mbapi_layer_t;

	endpoint_bind_t( const endpoint_bind_t & );
	void
	operator = ( const endpoint_bind_t & );

	//! Созданием сертификата может заниматься толькр слой mbapi.
	endpoint_bind_t(
		std::unique_ptr< impl::stagepoint_bind_impl_t > impl );

	public:
		virtual ~endpoint_bind_t();

		//! Получить наименование точки-стадии.
		const endpoint_t &
		endpoint() const;

		//! Отправка сообщения.
		template< class SERIALIZABLE >
		void
		send(
			//! Адресат.
			const endpoint_t & to,
			//! Сообщение.
			std::unique_ptr< SERIALIZABLE > & msg )
		{
			const oess_id_wrapper_t oess_id(
				&SERIALIZABLE::oess_serializer_t::type_name() );

			return send_impl(
				to,
				oess_id,
				std::unique_ptr< oess_2::stdsn::serializable_t >(
					msg.release() ) );
		}

		//! Отправка сообщения.
		template< class SERIALIZABLE >
		void
		send(
			//! Адресат.
			const endpoint_t & to,
			//! Сообщение.
			std::unique_ptr< SERIALIZABLE > && msg )
		{
			const oess_id_wrapper_t oess_id(
				&SERIALIZABLE::oess_serializer_t::type_name() );

			return send_impl(
				to,
				oess_id,
				std::unique_ptr< oess_2::stdsn::serializable_t >(
					msg.release() ) );
		}

		//! Отправка бинарное сообщение.
		void
		send_binary_message(
			//! Адресат.
			const endpoint_t & to,
			//! Тип сообщения.
			const oess_id_wrapper_t & oess_id,
			//! Бинарное сообщение.
			const std::string & binary_message );

	private:
		//! Выполнить отправку.
		void
		send_impl(
			//! Адресат.
			const endpoint_t & to,
			//! Тип сообщения.
			const oess_id_wrapper_t & oess_id,
			//! Сообщение.
			std::unique_ptr< oess_2::stdsn::serializable_t > msg );

		const endpoint_t m_endpoint;
};

//! Псевдоним unique_ptr для endpoint_bind_t
typedef std::unique_ptr< endpoint_bind_t > endpoint_bind_unique_ptr_t;

} /* namespace mbapi_4 */

#endif
