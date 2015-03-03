/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.4
 * \brief Private part of message limit implementation.
 */

#pragma once

#include <so_5/rt/h/message_limit.hpp>

#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>

namespace so_5
{

namespace rt
{

namespace message_limit
{

namespace impl
{

//
// info_block_t
//
/*!
 * \since v.5.5.4
 * \brief Run-time data for limit of one message type.
 */
struct info_block_t
	{
		//! Type of the message.
		std::type_index m_msg_type;

		//! Run-time data for the message type.
		control_block_t m_control_block;

		//! Initializing constructor.
		info_block_t(
			//! Type of the message.
			std::type_index msg_type,
			//! Limit for that message type.
			unsigned int limit,
			//! Reaction to the limit overflow.
			action_t action )
			:	m_msg_type( std::move( msg_type ) )
			,	m_control_block( limit, std::move( action ) )
			{}
	};

//
// info_block_container_t
//
/*!
 * \since v.5.5.4
 * \brief Type of container for limits' run-time data.
 */
using info_block_container_t = std::vector< info_block_t >;

//
// info_storage_t
//
/*!
 * \since v.5.5.4
 * \brief A storage for message limits for one agent.
 */
class info_storage_t
	{
		info_storage_t( const info_storage_t & ) = delete;
		info_storage_t & operator=( const info_storage_t & ) = delete;

	public :
		//! Initializing constructor.
		info_storage_t(
			//! Source description of limits.
			description_container_t && descriptions )
			:	m_blocks( build_blocks( std::move( descriptions ) ) )
			{}

		inline const control_block_t *
		find( const std::type_index & msg_type ) const
			{
				using namespace std;

//FIXME: for the big m_blocks a binary search must be used.
				// Simple linear search just for the first implementation.
				auto r = find_if( begin( m_blocks ), end( m_blocks ),
						[&]( const info_block_t & blk ) {
							return blk.m_msg_type == msg_type;
						} );
				if( r != end( m_blocks ) )
					return &(r->m_control_block);

				return nullptr;
			}

		//! Create info_storage object if there are some message limits.
		inline static std::unique_ptr< info_storage_t >
		create_if_necessary(
			description_container_t && descriptions )
			{
				std::unique_ptr< info_storage_t > result;

				if( !descriptions.empty() )
					result.reset(
							new info_storage_t( std::move( descriptions ) ) );

				return result;
			}

	private :
		//! Information about limits.
		const info_block_container_t m_blocks;

		//! Run-time limit information builder.
		inline static info_block_container_t
		build_blocks( description_container_t && descriptions )
			{
				using namespace std;

				info_block_container_t result;
				result.reserve( descriptions.size() );

				transform( begin( descriptions ), end( descriptions ),
						back_inserter( result ),
						[]( description_t & d ) {
							return info_block_t{
									d.m_msg_type,
									d.m_limit,
									std::move( d.m_action )
								};
						} );

				sort( begin( result ), end( result ),
						[]( const info_block_t & a, const info_block_t & b ) {
							return a.m_msg_type < b.m_msg_type;
						} );

				return result;
			}
	};

namespace
{

/*!
 * \since v.5.5.4
 * \brief Helper class to rollback message count change in
 * case of an exception.
 */
struct decrement_on_exception_t
{
	const control_block_t * m_limit;
	bool m_commited = false;

	decrement_on_exception_t( const control_block_t * limit )
		:	m_limit( limit )
	{}
	~decrement_on_exception_t()
	{
		if( !m_commited )
			--(m_limit->m_count);
	}

	void
	commit() { m_commited = true; }
};

} /* namespace anonymous */

/*!
 * \since v.5.5.4
 * \brief A helper function for pushing a message or a service
 * request to agent with respect to message limit.
 *
 * \tparam INVOCATION_TYPE it is a message or service request.
 * \tparam LAMBDA lambda-function to do actual pushing.
 */
template< so_5::rt::invocation_type_t INVOCATION_TYPE, typename LAMBDA >
void
try_to_deliver_to_agent(
	//! Receiver of the message or service request.
	const agent_t & receiver,
	//! Optional message limit.
	//! Value nullptr means that there is no message limit to control.
	const control_block_t * limit,
	//! Type of message to be delivered.
	const std::type_index & msg_type,
	//! Message instance to be delivered.
	const message_ref_t & what_to_deliver,
	//! Deep of overlimit reactions recursion.
	unsigned int overlimit_reaction_deep,
	//! Actual delivery action.
	LAMBDA delivery_action )
{
	if( limit && ( limit->m_limit < ++(limit->m_count) ) )
	{
		--(limit->m_count);

		limit->m_action(
			overlimit_context_t{
				receiver,
				INVOCATION_TYPE,
				overlimit_reaction_deep,
				msg_type,
				what_to_deliver } );
	}
	else
	{
		decrement_on_exception_t exception_guard{ limit };

		delivery_action();

		exception_guard.commit();
	}
}

} /* namespace impl */

} /* namespace message_limit */

} /* namespace rt */

} /* namespace so_5 */

