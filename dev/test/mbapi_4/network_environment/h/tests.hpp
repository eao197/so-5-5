/*
	Тестирование mbapi_4::impl::network_environment_t.
*/

#if !defined( _TEST__MBAPI_4__NETWORK_ENVIRONMENT__TESTS_HPP_ )
#define _TEST__MBAPI_4__NETWORK_ENVIRONMENT__TESTS_HPP_

#include <set>
#include <vector>

#include <utest_helper_1/h/helper.hpp>

#include <mbapi_4/impl/infrastructure/h/network_environment.hpp>

using namespace mbapi_4;
using namespace mbapi_4::impl::infrastructure;

//
// Макросы для задания конечных точек.
//

#define ENDPOINTS_ADD_BEGIN( endpoints ) \
	{ \
		available_endpoint_table_t & _available_endpoint_table_ = endpoints;

#define ENDPOINT_DESC_BEGIN( name, channel_id, node_uid, distance ) \
	{ \
		endpoint_t _ep_( name ); \
		available_endpoint_ptr_t _aep_ptr_( new available_endpoint_t ); \
		_aep_ptr_->m_channel_id = channel_id; \
		_aep_ptr_->m_node_uid = mbapi_node_uid_t::utest_create_uid( node_uid ); \
		_aep_ptr_->m_distance = distance;

#define ENDPOINT_NO_STAGES const endpoint_stage_chain_t ep_stage_chain( _ep_ );

struct stage_appender_t
{
	stage_appender_t(
		std::vector< std::string > & v )
		:
			m_v( v )
	{}

	inline stage_appender_t &
	operator << ( const std::string & stage )
	{
		m_v.push_back( stage );
		return *this;
	}

	std::vector< std::string > & m_v;
};

#define ENDPOINT_STAGES( stage_chain ) \
	std::vector< std::string > _stages_; \
	{ stage_appender_t _appender_( _stages_ ); _appender_ << stage_chain; } \
	const endpoint_stage_chain_t ep_stage_chain( _ep_,  _stages_ );

#define ENDPOINT_DESC_END() \
	_aep_ptr_->m_endpoint_stage_chain = ep_stage_chain; \
	_available_endpoint_table_[ _ep_ ] = _aep_ptr_; \
	}

#define ENDPOINTS_ADD_END() \
	}

//
// Макросы для задания точек-стадий.
//

#define STAGEPOINTS_ADD_BEGIN( stagepoints ) \
	{ \
		available_stagepoint_table_t & _stagepoints_ = stagepoints; \

#define STAGEPOINT_DESC( name, ep_name, channel_id, node_uid, distance ) \
	{ \
		stagepoint_t _sp_( name, ep_name ); \
		available_stagepoint_ptr_t _asp_ptr_( new available_stagepoint_t ); \
		_asp_ptr_->m_channel_id = channel_id; \
		_asp_ptr_->m_node_uid = mbapi_node_uid_t::utest_create_uid( node_uid ); \
		_asp_ptr_->m_distance = distance; \
		_stagepoints_[ _sp_ ] = _asp_ptr_; \
	}

#define STAGEPOINTS_ADD_END() \
	}

//
// Проверка конечных точек.
//

#define ENDPOINTS_CHECK_BEGIN( ep_table, el ) \
	{ \
		available_endpoint_table_t _aep_ = ep_table; \
		endpoint_list_t _el_ = el; \

#define ENDPOINT_CHECK_BEGIN( name, channel_id, node_uid, distance ) \
	{ \
		const std::string _ep_name_( name ); \
		available_endpoint_table_t::iterator it = _aep_.find( endpoint_t( _ep_name_ ) ); \
		UT_CHECK_CONDITION( _aep_.end() != it ); \
		available_endpoint_t & p = *(it->second); \
		UT_CHECK_EQ( channel_id.str(), p.m_channel_id.str() ); \
		UT_CHECK_EQ( node_uid, p.m_node_uid.str() ); \
		UT_CHECK_EQ( distance, p.m_distance ); \
		{\
			endpoint_list_t::iterator it = \
				std::find( _el_.begin(), _el_.end(), _ep_name_ ); \
			UT_CHECK_CONDITION( _el_.end() != it ); \
			_el_.erase( it ); \
		}


#define ENDPOINT_CHECK_NO_STAGES \
		UT_CHECK_EQ( 0, p.m_endpoint_stage_chain.stages().size() );

#define ENDPOINT_CHECK_STAGES( stage_chain ) \
	std::vector< std::string > _stages_; \
	{ stage_appender_t _appender_( _stages_ ); _appender_ << stage_chain; } \
	UT_CHECK_EQ( _stages_.size(), p.m_endpoint_stage_chain.stages().size() ); \
	for( size_t i = 0; i < _stages_.size(); ++i ){ \
		UT_PUSH_CONTEXT( "Check endpoints" ) << " i=" << i; \
		UT_CHECK_EQ( _stages_[ i ], p.m_endpoint_stage_chain.stages()[ i ] ); \
		UT_POP_CONTEXT(); \
	}

#define ENDPOINT_CHECK_END() \
	_aep_.erase( it ); \
	}

#define ENDPOINTS_CHECK_END() \
	UT_CHECK_CONDITION( _aep_.empty() ); \
	UT_CHECK_CONDITION( _el_.empty() ); \
	}


//
// Проверка точек-стадий.
//

#define STAGEPOINTS_CHECK_BEGIN( stagepoints ) \
	{ \
		available_stagepoint_table_t _stagepoints_ = stagepoints; \

#define STAGEPOINT_CHECK( name, ep_name, channel_id, node_uid, distance ) \
	{ \
		stagepoint_t _sp_( name, ep_name ); \
		available_stagepoint_table_t::iterator it = _stagepoints_.find( _sp_ ); \
		UT_CHECK_CONDITION( _stagepoints_.end() != it ); \
		available_stagepoint_t & p = *(it->second); \
		UT_CHECK_EQ( channel_id.str(), p.m_channel_id.str() ); \
		UT_CHECK_EQ( node_uid, p.m_node_uid.str() ); \
		UT_CHECK_EQ( distance, p.m_distance ); \
		_stagepoints_.erase( it ); \
	}

#define STAGEPOINTS_CHECK_END() \
	UT_CHECK_CONDITION( _stagepoints_.empty() ); \
	}


//
// Проверка каналов
//

void
check_endpoints(
	std::set< endpoint_t > & expected_endpoints,
	const endpoint_list_t & endpoint_list,
	UT_CONTEXT_DECL );

#define EXP_ENDPOINT_CHECK_BEGIN() \
	{ \
		const endpoint_list_t & _endpoints_ = c.endpoints();\
		std::set< endpoint_t > _expected_endpoints_; \

#define EXP_ENDPOINT( endpoint_name ) \
		_expected_endpoints_.insert( endpoint_t( endpoint_name ) );

#define EXP_ENDPOINT_CHECK_END() \
		UT_CHECK_EQ( _expected_endpoints_.size(), _endpoints_.size() ); \
		UT_PUSH_CONTEXT( "Check endpoints" ); \
		check_endpoints( _expected_endpoints_, _endpoints_, UT_CONTEXT ); \
		UT_POP_CONTEXT(); \
	}

void
check_stages(
	std::set< stagepoint_t > & expected_stages,
	const stagepoint_list_t & stagepoint_list,
	UT_CONTEXT_DECL );

#define EXP_STAGE_CHECK_BEGIN() \
	{ \
		const stagepoint_list_t & _stages_ = c.stagepoints(); \
		std::set< stagepoint_t > _expected_stages_; \

#define EXP_STAGE( stage_name, endpoint_name ) \
		_expected_stages_.insert( stagepoint_t( stage_name, endpoint_name ) );

#define EXP_STAGE_CHECK_END() \
		UT_CHECK_EQ( _expected_stages_.size(), c.stagepoints().size() ); \
		UT_PUSH_CONTEXT( "Check stages" ); \
		check_stages( _expected_stages_, _stages_, UT_CONTEXT ); \
		UT_POP_CONTEXT(); \
	}

#define CHANNELS_CHECK_BEGIN( channel_info_table ) \
	{ \
		channel_info_table_t & _channel_info_table_ = channel_info_table;

#define CHANNEL_CHECK_BEGIN( channel_id, _uid_ ) \
	{ \
		channel_info_table_t::iterator it = _channel_info_table_.find( channel_id ); \
		UT_CHECK_CONDITION( _channel_info_table_.end() != it ); \
		channel_info_t & c = *(it->second); \
		UT_CHECK_EQ( _uid_, c.node_uid().str() );

#define CHANNEL_CHECK_END() \
	}

#define CHANNELS_CHECK_END() \
	}

//
// Тесты.
//

void
single_channel_updates_test_impl(
	UT_CONTEXT_DECL );

void
multiple_channel_updates_test_impl(
	UT_CONTEXT_DECL );

void
local_node_updates_test_impl(
	UT_CONTEXT_DECL );

void
shift_to_next_stage_test_impl(
	UT_CONTEXT_DECL );

#endif
