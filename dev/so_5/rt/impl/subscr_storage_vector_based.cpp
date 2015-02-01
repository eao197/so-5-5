/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.3
 * \file
 * \brief A vector-based storage for agent's subscriptions information.
 */

#include <so_5/rt/impl/h/subscription_storage_iface.hpp>

#include <algorithm>
#include <vector>
#include <sstream>

namespace so_5
{

namespace rt
{

namespace impl
{

/*!
 * \since v.5.5.3
 * \brief A vector-based storage for agent's subscriptions information.
 */
namespace vector_based_subscr_storage
{

namespace
{
//FIXME: this method must be defined as reusable
//(may be in subscription_storage_iface.hpp).
	std::string
	make_subscription_description(
		const mbox_t & mbox_ref,
		std::type_index msg_type,
		const state_t & state )
	{
		std::ostringstream s;
		s << "(mbox:'" << mbox_ref->query_name()
			<< "', msg_type:'" << msg_type.name() << "', state:'"
			<< state.query_name() << "')";

		return s.str();
	}

} /* namespace anonymous */

/*!
 * \since v.5.5.3
 * \brief A vector-based storage for agent's subscriptions information.
 *
 * This is very simple implementation of subscription storage which
 * uses std::vector for storing information.
 *
 * All manipulation is performed by very simple linear search inside
 * that vector. For agents with few subscriptions this will be the most
 * efficient approach.
 */
class storage_t : public subscription_storage_t
	{
	public :
//FIXME: must receive initial vector size.
		storage_t( agent_t * owner );
		~storage_t();

		virtual void
		create_event_subscription(
			const mbox_t & mbox_ref,
			std::type_index type_index,
			const state_t & target_state,
			const event_handler_method_t & method,
			thread_safety_t thread_safety ) override;

		virtual void
		drop_subscription(
//FIXME: change order of arguments: mbox_ref, type_index
			const std::type_index & type_index,
			const mbox_t & mbox_ref,
			const state_t & target_state ) override;

		void
		drop_subscription_for_all_states(
//FIXME: change order of arguments: mbox_ref, type_index
			const std::type_index & type_index,
			const mbox_t & mbox_ref );

		const event_handler_data_t *
		find_handler(
			mbox_id_t mbox_id,
			const std::type_index & msg_type,
			const state_t & current_state ) const;

		void
		debug_dump( std::ostream & to ) const;

	private :
		//! Information about subscription.
		struct info_t
			{
				//! Reference to mbox.
				/*!
				 * Reference must be stored because we must have
				 * access to mbox during destroyment of all
				 * subscriptions in destructor.
				 */
				mbox_t m_mbox;
				std::type_index m_msg_type;
				const state_t * m_state;
				event_handler_data_t m_handler;

				info_t(
					mbox_t mbox,
					std::type_index msg_type,
					const state_t & state,
					const event_handler_method_t & method,
					thread_safety_t thread_safety )
					:	m_mbox( std::move( mbox ) )
					,	m_msg_type( std::move( msg_type ) )
					,	m_state( &state )
					,	m_handler( method, thread_safety )
					{}
			};

		//! Type of vector with subscription information.
		typedef std::vector< info_t > subscr_info_vector_t;

		//! Subscription information.
		subscr_info_vector_t m_events;

		void
		destroy_all_subscriptions();
	};

namespace
{
	template< class Container >
	auto
	find( Container & c,
		const mbox_id_t & mbox_id,
		const std::type_index & msg_type,
		const state_t & target_state ) -> decltype( c.begin() )
		{
			using namespace std;

			return find_if( begin( c ), end( c ),
				[&]( typename Container::value_type const & o ) {
					return ( o.m_mbox->id() == mbox_id &&
						o.m_msg_type == msg_type &&
						o.m_state == &target_state );
				} );
		}

} /* namespace anonymous */

storage_t::storage_t( agent_t * owner )
	:	subscription_storage_t( owner )
	{}

storage_t::~storage_t()
	{
		destroy_all_subscriptions();
	}

void
storage_t::create_event_subscription(
	const mbox_t & mbox,
	std::type_index msg_type,
	const state_t & target_state,
	const event_handler_method_t & method,
	thread_safety_t thread_safety )
	{
		const auto mbox_id = mbox->id();

		// Check that this subscription is new.
		auto existed_position = find(
				m_events,
				mbox_id, msg_type, target_state );

		if( existed_position != m_events.end() )
			SO_5_THROW_EXCEPTION(
				rc_evt_handler_already_provided,
				"agent is already subscribed to message, " +
				make_subscription_description( mbox, msg_type, target_state ) );

		// Just add subscription to the end.
		m_events.emplace_back(
				mbox, msg_type, target_state, method, thread_safety );

//FIXME: this step is not necessary if mbox is a direct mbox!
		// If there is no subscription for that mbox it must be created.
		for( std::size_t i = 0, max = m_events.size() - 1; i != max; ++i )
			if( mbox_id == m_events[ i ].m_mbox->id() &&
					msg_type == m_events[ i ].m_msg_type )
				// Mbox already knows about this agents.
				// There is no need to continue.
				return;

		// Mbox must create subscription.
		try
			{
				mbox->subscribe_event_handler( msg_type, owner() );
			}
		catch( ... )
			{
				// Rollback agent's subscription.
				m_events.pop_back();
				throw;
			}
	}

void
storage_t::drop_subscription(
	const std::type_index & msg_type,
	const mbox_t & mbox,
	const state_t & target_state )
	{
		const auto mbox_id = mbox->id();

		auto existed_position = find(
				m_events,
				mbox_id, msg_type, target_state );
		if( existed_position != m_events.end() )
			{
				m_events.erase( existed_position );

				// If there is no more subscriptions to that mbox
				// mbox must remove information about that agent.
//FIXME: this step is not necessary if mbox is a direct mbox!
				for( const auto & e : m_events )
					if( mbox_id == e.m_mbox->id() &&
							msg_type == e.m_msg_type )
						return;

				// If we are here then there is no more references
				// to the mbox. And mbox must not hold reference
				// to the agent.
				mbox->unsubscribe_event_handlers( msg_type, owner() );
			}
	}

void
storage_t::drop_subscription_for_all_states(
	const std::type_index & msg_type,
	const mbox_t & mbox )
	{
		using namespace std;

		const auto mbox_id = mbox->id();

		const auto old_size = m_events.size();

		m_events.erase(
				remove_if( begin( m_events ), end( m_events ),
						[mbox_id, &msg_type]( const info_t & i ) {
							return i.m_mbox->id() == mbox_id &&
									i.m_msg_type == msg_type;
						} ),
				end( m_events ) );

//FIXME: must be removed later.
#if 0
		for( auto it = m_events.begin(); it != m_events.end(); ++it )
			if( mbox_id == it->m_mbox->id() )
				it = m_events.erase( it );
#endif

//FIXME: this step is not necessary if mbox is a direct mbox!
		if( old_size != m_events.size() )
			mbox->unsubscribe_event_handlers( msg_type, owner() );
	}

const event_handler_data_t *
storage_t::find_handler(
	mbox_id_t mbox_id,
	const std::type_index & msg_type,
	const state_t & current_state ) const
	{
		auto it = find(
				m_events,
				mbox_id, msg_type, current_state );

		if( it != std::end( m_events ) )
			return &(it->m_handler);
		else
			return nullptr;
	}

void
storage_t::debug_dump( std::ostream & to ) const
	{
		for( const auto & e : m_events )
			std::cout << "{" << e.m_mbox->id() << ", "
					<< e.m_msg_type.name() << ", "
					<< e.m_state->query_name() << "}"
					<< std::endl;
	}

void
storage_t::destroy_all_subscriptions()
	{
		if( m_events.empty() )
			// Nothing to do at empty subscription list.
			return;

		using namespace std;

		struct mbox_msg_type_pair_t
			{
				abstract_message_box_t * m_mbox;
				const type_index * m_msg_type;

				bool
				operator<( const mbox_msg_type_pair_t & o ) const
					{
						return m_mbox < o.m_mbox ||
								( m_mbox == o.m_mbox &&
								 (*m_msg_type) < (*o.m_msg_type) );
					}

				bool
				operator==( const mbox_msg_type_pair_t & o ) const
					{
						return m_mbox == o.m_mbox &&
								(*m_msg_type) == (*o.m_msg_type);
					}
			};

		// First step: collect all pointers to mbox-es.
		vector< mbox_msg_type_pair_t > mboxes;
		mboxes.reserve( m_events.size() );

		transform(
				begin( m_events ), end( m_events ),
				back_inserter( mboxes ),
				[]( info_t & i ) {
					return mbox_msg_type_pair_t{ i.m_mbox.get(), &i.m_msg_type };
				} );

		// Second step: remove duplicates.
		sort( begin( mboxes ), end( mboxes ) );
		mboxes.erase(
				unique( begin( mboxes ), end( mboxes ) ),
				end( mboxes ) );

		// Third step: destroy subscription in mboxes.
		for( auto m : mboxes )
			m.m_mbox->unsubscribe_event_handlers( *m.m_msg_type, owner() );

		// Fourth step: cleanup subscription vector.
		subscr_info_vector_t empty_events;
		m_events.swap( empty_events );
	}

} /* namespace vector_based_subscr_storage */

} /* namespace impl */

SO_5_FUNC subscription_storage_factory_t
vector_based_subscription_storage_factory()
	{
		return []( agent_t * owner ) {
			return impl::subscription_storage_unique_ptr_t(
					new impl::vector_based_subscr_storage::storage_t( owner ) );
		};
	}

} /* namespace rt */

} /* namespace so_5 */


