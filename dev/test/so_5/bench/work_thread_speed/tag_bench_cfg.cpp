// tags generated file (implementation)
#include "tag_bench_cfg.hpp"

//
// tag_agent_by_active_group_t
//

// destructor
tag_agent_by_active_group_t::~tag_agent_by_active_group_t()
{
}

// Standart constructor.
tag_agent_by_active_group_t::tag_agent_by_active_group_t(
	const std::string & name,
	bool is_mandatory )
	:
		base_type_t( 
			name, 
			is_mandatory ),
		m_active_group( self_tag(), "active_group", true )
{
}

// In case tag is a subtag.
tag_agent_by_active_group_t::tag_agent_by_active_group_t(
	cls_3::tags::tag_t & owner,
	const std::string & name,
	bool is_mandatory )
	:
		base_type_t( 
			owner, 
			name, 
			is_mandatory ),
		m_active_group( self_tag(), "active_group", true )
{
}

//
// tag_bench_pair_t
//

// destructor
tag_bench_pair_t::~tag_bench_pair_t()
{
}

// Standart constructor.
tag_bench_pair_t::tag_bench_pair_t(
	const std::string & name,
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			name, 
			is_mandatory,
			is_one_copy_only ),
		m_sender( self_tag(), "sender", true ),
		m_receiver( self_tag(), "receiver", true ),
		m_message_count( self_tag(), "message_count", true ),
		m_message_count_min_max_constraint( 1, 1000000000 )
{
	m_message_count.set_constraint( &m_message_count_min_max_constraint );
}

// In case tag is a subtag.
tag_bench_pair_t::tag_bench_pair_t(
	cls_3::tags::tag_t & owner,
	const std::string & name,
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			owner, 
			name, 
			is_mandatory,
			is_one_copy_only ),
		m_sender( self_tag(), "sender", true ),
		m_receiver( self_tag(), "receiver", true ),
		m_message_count( self_tag(), "message_count", true ),
		m_message_count_min_max_constraint( 1, 1000000000 )
{
	m_message_count.set_constraint( &m_message_count_min_max_constraint );
}

// With predefined cls-name "bench_pair"
tag_bench_pair_t::tag_bench_pair_t(
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			"bench_pair", 
			is_mandatory,
			is_one_copy_only ),
		m_sender( self_tag(), "sender", true ),
		m_receiver( self_tag(), "receiver", true ),
		m_message_count( self_tag(), "message_count", true ),
		m_message_count_min_max_constraint( 1, 1000000000 )
{
	m_message_count.set_constraint( &m_message_count_min_max_constraint );
}

// With predefined cls-name "bench_pair"
// and tag is a subtag.
tag_bench_pair_t::tag_bench_pair_t(
	cls_3::tags::tag_t & owner,
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			owner, 
			"bench_pair", 
			is_mandatory,
			is_one_copy_only ),
		m_sender( self_tag(), "sender", true ),
		m_receiver( self_tag(), "receiver", true ),
		m_message_count( self_tag(), "message_count", true ),
		m_message_count_min_max_constraint( 1, 1000000000 )
{
	m_message_count.set_constraint( &m_message_count_min_max_constraint );
}

//
// tag_wt_bench_t
//

// destructor
tag_wt_bench_t::~tag_wt_bench_t()
{
}

// Standart constructor.
tag_wt_bench_t::tag_wt_bench_t(
	const std::string & name,
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			name, 
			is_mandatory,
			is_one_copy_only ),
		m_bench_pair( self_tag(), "bench_pair", true ),
		m_msg_blocks_size( self_tag(), "msg_blocks_size", false, false ),
		m_msg_blocks_size_min_max_constraint( 1, 1000000000 )
{
	m_msg_blocks_size.set_constraint( &m_msg_blocks_size_min_max_constraint );
}

// In case tag is a subtag.
tag_wt_bench_t::tag_wt_bench_t(
	cls_3::tags::tag_t & owner,
	const std::string & name,
	bool is_mandatory,
	bool is_one_copy_only )
	:
		base_type_t( 
			owner, 
			name, 
			is_mandatory,
			is_one_copy_only ),
		m_bench_pair( self_tag(), "bench_pair", true ),
		m_msg_blocks_size( self_tag(), "msg_blocks_size", false, false ),
		m_msg_blocks_size_min_max_constraint( 1, 1000000000 )
{
	m_msg_blocks_size.set_constraint( &m_msg_blocks_size_min_max_constraint );
}
