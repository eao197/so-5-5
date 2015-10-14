/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.9
 * \file
 * \brief Various helpers for message delivery tracing stuff.
 */

#pragma once

#include <so_5/h/msg_tracing.hpp>

#include <so_5/rt/h/mbox.hpp>

#include <sstream>

namespace so_5 {

namespace rt {

namespace impl {

namespace msg_tracing_helpers {

//
// tracing_disabled_base_t
//
/*!
 * \since v.5.5.9
 * \brief Base class for an mbox for the case when message delivery
 * tracing is disabled.
 */
struct tracing_disabled_base_t
	{
		void
		trace_subscribe_event_handler(
			const abstract_message_box_t &,
			const std::type_index &,
			const so_5::rt::message_limit::control_block_t *,
			const agent_t * ) const
			{
				// No implementation. This method must be removed by
				// optimized compiler.
			}

		void
		trace_unsubscribe_event_handler(
			const abstract_message_box_t &,
			const std::type_index &,
			const agent_t * ) const
			{
				// No implementation. This method must be removed by
				// optimized compiler.
			}

		void
		trace_set_delivery_filter(
			const abstract_message_box_t &,
			const std::type_index &,
			const agent_t * ) const
			{
				// No implementation. This method must be removed by
				// optimized compiler.
			}

		void
		trace_drop_delivery_filter(
			const abstract_message_box_t &,
			const std::type_index &,
			const agent_t * ) const
			{
				// No implementation. This method must be removed by
				// optimized compiler.
			}

		class deliver_op_tracer_t
			{
			public :
				deliver_op_tracer_t(
					const tracing_disabled_base_t &,
					const abstract_message_box_t &,
					const char *,
					const std::type_index &,
					const message_ref_t &,
					const unsigned int )
					{}

				void
				commit() {}

				void
				no_subscribers() const {}

				void
				delivery_attempt( const agent_t * ) const {}

				void
				push_to_queue( const agent_t * ) const {}

				void
				message_rejected(
					const agent_t *,
					const delivery_possibility_t ) const {}
			};
//FIXME: must be implemented!
	};

//
// tracing_enabled_base_t
//
/*!
 * \since v.5.5.9
 * \brief Base class for an mbox for the case when message delivery
 * tracing is enabled.
 */
class tracing_enabled_base_t
	{
	private :
		so_5::msg_tracing::tracer_t & m_tracer;

	public :
		tracing_enabled_base_t( so_5::msg_tracing::tracer_t & tracer )
			:	m_tracer{ tracer }
			{}

		so_5::msg_tracing::tracer_t &
		tracer() const
			{
				return m_tracer;
			}

		void
		trace_subscribe_event_handler(
			const abstract_message_box_t & mbox,
			const std::type_index & msg_type,
			const so_5::rt::message_limit::control_block_t * limit,
			const agent_t * subscriber ) const
			{
				std::ostringstream s;

				s << "msg_trace [tid=" << query_current_thread_id()
					<< "][mbox_id=" << mbox.id()
					<< "][mbox_name=" << mbox.query_name()
					<< "] subscribe_event_handler [msg_type=" << msg_type.name()
					<< "][agent_ptr=" << subscriber
					<< "][limit_ptr=" << limit << "]";

				m_tracer.trace( s.str() );
			}

		void
		trace_unsubscribe_event_handler(
			const abstract_message_box_t & mbox,
			const std::type_index & msg_type,
			const agent_t * subscriber ) const
			{
				std::ostringstream s;

				s << "msg_trace [tid=" << query_current_thread_id()
					<< "][mbox_id=" << mbox.id()
					<< "][mbox_name=" << mbox.query_name()
					<< "] unsubscribe_event_handler [msg_type=" << msg_type.name()
					<< "][agent_ptr=" << subscriber << "]";

				m_tracer.trace( s.str() );
			}

		void
		trace_set_delivery_filter(
			const abstract_message_box_t & mbox,
			const std::type_index & msg_type,
			const agent_t * subscriber ) const
			{
				std::ostringstream s;

				s << "msg_trace [tid=" << query_current_thread_id()
					<< "][mbox_id=" << mbox.id()
					<< "][mbox_name=" << mbox.query_name()
					<< "] set_delivery_filter [msg_type=" << msg_type.name()
					<< "][agent_ptr=" << subscriber << "]";

				m_tracer.trace( s.str() );
			}

		void
		trace_drop_delivery_filter(
			const abstract_message_box_t & mbox,
			const std::type_index & msg_type,
			const agent_t * subscriber ) const
			{
				std::ostringstream s;

				s << "msg_trace [tid=" << query_current_thread_id()
					<< "][mbox_id=" << mbox.id()
					<< "][mbox_name=" << mbox.query_name()
					<< "] drop_delivery_filter [msg_type=" << msg_type.name()
					<< "][agent_ptr=" << subscriber << "]";

				m_tracer.trace( s.str() );
			}

		class deliver_op_tracer_t
			{
			private :
				so_5::msg_tracing::tracer_t & m_tracer;
				const abstract_message_box_t & m_mbox;
				const char * m_op_name;
				const std::type_index & m_msg_type;
				const message_ref_t & m_message;
				const unsigned int m_overlimit_reaction_deep;
				bool m_commited = false;

			public :
				deliver_op_tracer_t(
					const tracing_enabled_base_t & tracing_base,
					const abstract_message_box_t & mbox,
					const char * op_name,
					const std::type_index & msg_type,
					const message_ref_t & message,
					const unsigned int overlimit_reaction_deep )
					:	m_tracer{ tracing_base.tracer() }
					,	m_mbox{ mbox }
					,	m_op_name{ op_name }
					,	m_msg_type{ msg_type }
					,	m_message{ message }
					,	m_overlimit_reaction_deep{ overlimit_reaction_deep }
					{
						std::ostringstream s;

						s << "msg_trace [tid=" << query_current_thread_id()
							<< "][mbox_id=" << m_mbox.id()
							<< "][mbox_name=" << m_mbox.query_name()
							<< "] " << m_op_name << ".started "
							<< "[msg_type=" << m_msg_type.name()
							<< "][msg_ptr=" << m_message.get()
							<< "][overlimit_deep=" << m_overlimit_reaction_deep << "]";

						m_tracer.trace( s.str() );
					}

				~deliver_op_tracer_t()
					{
						const char * const result =
								m_commited ? ".finished" : ".aborted";

						std::ostringstream s;

						s << "msg_trace [tid=" << query_current_thread_id()
							<< "][mbox_id=" << m_mbox.id()
							<< "][mbox_name=" << m_mbox.query_name()
							<< "] " << m_op_name << result << " "
							<< "[msg_type=" << m_msg_type.name()
							<< "][msg_ptr=" << m_message.get()
							<< "][overlimit_deep=" << m_overlimit_reaction_deep << "]";

						m_tracer.trace( s.str() );
					}

				void
				commit() { m_commited = true; }

				void
				no_subscribers() const
					{
						std::ostringstream s;

						s << "msg_trace [tid=" << query_current_thread_id()
							<< "][mbox_id=" << m_mbox.id()
							<< "][mbox_name=" << m_mbox.query_name()
							<< "] " << m_op_name << ".no_subscribers "
							<< "[msg_type=" << m_msg_type.name()
							<< "][msg_ptr=" << m_message.get()
							<< "][overlimit_deep=" << m_overlimit_reaction_deep
							<< "]";

						m_tracer.trace( s.str() );
					}

				void
				delivery_attempt( const agent_t * subscriber ) const
					{
						std::ostringstream s;

						s << "msg_trace [tid=" << query_current_thread_id()
							<< "][mbox_id=" << m_mbox.id()
							<< "][mbox_name=" << m_mbox.query_name()
							<< "] " << m_op_name << ".delivery_attempt "
							<< "[msg_type=" << m_msg_type.name()
							<< "][msg_ptr=" << m_message.get()
							<< "][overlimit_deep=" << m_overlimit_reaction_deep
							<< "][agent_ptr=" << subscriber << "]";

						m_tracer.trace( s.str() );
					}

				void
				push_to_queue( const agent_t * subscriber ) const
					{
						std::ostringstream s;

						s << "msg_trace [tid=" << query_current_thread_id()
							<< "][mbox_id=" << m_mbox.id()
							<< "][mbox_name=" << m_mbox.query_name()
							<< "] " << m_op_name << ".push_to_queue "
							<< "[msg_type=" << m_msg_type.name()
							<< "][msg_ptr=" << m_message.get()
							<< "][overlimit_deep=" << m_overlimit_reaction_deep
							<< "][agent_ptr=" << subscriber << "]";

						m_tracer.trace( s.str() );
					}

				void
				message_rejected(
					const agent_t * subscriber,
					const delivery_possibility_t status ) const
					{
						if( delivery_possibility_t::disabled_by_delivery_filter
								== status )
							{
								std::ostringstream s;

								s << "msg_trace [tid=" << query_current_thread_id()
									<< "][mbox_id=" << m_mbox.id()
									<< "][mbox_name=" << m_mbox.query_name()
									<< "] " << m_op_name << ".message_rejected "
									<< "[msg_type=" << m_msg_type.name()
									<< "][msg_ptr=" << m_message.get()
									<< "][overlimit_deep=" << m_overlimit_reaction_deep
									<< "][agent_ptr=" << subscriber << "]";

								m_tracer.trace( s.str() );
							}
					}

			};

//FIXME: must be implemented!
	};

} /* namespace msg_tracing_helpers */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */


