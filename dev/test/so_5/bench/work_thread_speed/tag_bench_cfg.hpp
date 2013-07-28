// tags generated file (header)

#if !defined( _GENERATED_____BENCH_CFG )
#define _GENERATED_____BENCH_CFG

#include <cls_3/tags/h/tag_no_value.hpp>
#include <cls_3/tags/scalar/h/tag_scalar.hpp>
#include <cls_3/tags/scalar/h/tag_scalar_vector.hpp>
#include <cls_3/tags/h/tag_vector_of_tags.hpp>
#include <cls_3/tags/scalar/constraints/h/min_max.hpp>
#include <cls_3/tags/scalar/constraints/h/one_of.hpp>


//
// tag_agent_by_active_group_t
//

class tag_agent_by_active_group_t
	:
		public cls_3::tags::tag_scalar_t< std::string >
{
		typedef cls_3::tags::tag_scalar_t< std::string > base_type_t;
	public:
		//! Standart constructor.
		tag_agent_by_active_group_t(
			const std::string & name,
			bool is_mandatory );

		//! In case tag is a subtag.
		tag_agent_by_active_group_t(
			cls_3::tags::tag_t & owner,
			const std::string & name,
			bool is_mandatory );

		virtual ~tag_agent_by_active_group_t();

		
		// Attributes:
		cls_3::tags::tag_scalar_t< std::string > m_active_group;

private:
};

//
// tag_bench_pair_t
//

class tag_bench_pair_t
	:
		public cls_3::tags::tag_no_value_t
{
		typedef cls_3::tags::tag_no_value_t base_type_t;
	public:
		//! Standart constructor.
		tag_bench_pair_t(
			const std::string & name,
			bool is_mandatory,
			bool is_one_copy_only = false );

		//! In case tag is a subtag.
		tag_bench_pair_t(
			cls_3::tags::tag_t & owner,
			const std::string & name,
			bool is_mandatory,
			bool is_one_copy_only = false );

		//! With predefined cls-name "bench_pair"
		tag_bench_pair_t(
			bool is_mandatory,
			bool is_one_copy_only = false );

		//! With predefined cls-name "bench_pair"
		//! and tag is a subtag.
		tag_bench_pair_t(
			cls_3::tags::tag_t & owner,
			bool is_mandatory,
			bool is_one_copy_only = false );

		virtual ~tag_bench_pair_t();

		// Subtags:
		tag_agent_by_active_group_t m_sender;
		tag_agent_by_active_group_t m_receiver;

		// Attributes:
		cls_3::tags::tag_scalar_t< unsigned int > m_message_count;

private:
		cls_3::scalar_constraints::min_max_t<unsigned int > m_message_count_min_max_constraint;
};

//
// tag_wt_bench_t
//

class tag_wt_bench_t
	:
		public cls_3::tags::tag_no_value_t
{
		typedef cls_3::tags::tag_no_value_t base_type_t;
	public:
		//! Standart constructor.
		tag_wt_bench_t(
			const std::string & name,
			bool is_mandatory,
			bool is_one_copy_only = false );

		//! In case tag is a subtag.
		tag_wt_bench_t(
			cls_3::tags::tag_t & owner,
			const std::string & name,
			bool is_mandatory,
			bool is_one_copy_only = false );

		virtual ~tag_wt_bench_t();

		// Subtags:
		cls_3::tags::tag_vector_of_tags_t< tag_bench_pair_t > m_bench_pair;

		// Attributes:
		cls_3::tags::tag_scalar_vector_t< unsigned int > m_msg_blocks_size;

private:
		cls_3::scalar_constraints::min_max_t<unsigned int > m_msg_blocks_size_min_max_constraint;
};

#endif // _GENERATED_____BENCH_CFG

