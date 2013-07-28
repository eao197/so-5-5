/*
	MBAPI 4 Test Bench
*/

/*!
	Типы передаваемых сообщений.
*/

#if !defined( _TEST__MBAPI_4__BENCH__MESSAGES_HPP_ )
#define _TEST__MBAPI_4__BENCH__MESSAGES_HPP_

#include <string>

#include <oess_2/defs/h/types.hpp>
#include <oess_2/stdsn/h/serializable.hpp>

namespace bench_mbapi_4
{

struct request_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( request_t )

	public:
		request_t()
		{}

		request_t( size_t num )
			:
				m_num ( num )
		{}

		//! Номер.
		oess_2::uint_t m_num;
};

struct start_test_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( start_test_t )

	public:
};

struct finish_test_t
	:
		public oess_2::stdsn::serializable_t
{
		OESS_SERIALIZER( finish_test_t )

	public:
};

} /* namespace bench_mbapi_4 */

#endif
