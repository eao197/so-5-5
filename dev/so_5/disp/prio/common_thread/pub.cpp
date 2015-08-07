/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.8
 * \file
 * \brief Functions for creating and binding of the single thread dispatcher
 * with priority support.
 */

#include <so_5/disp/prio/common_thread/h/pub.hpp>

#include <so_5/disp/prio/common_thread/impl/h/demand_queue.hpp>
#include <so_5/disp/prio/common_thread/impl/h/work_thread.hpp>

#include <so_5/disp/reuse/h/disp_binder_helpers.hpp>

namespace so_5 {

namespace disp {

namespace prio {

namespace common_thread {

namespace impl {

//
// dispatcher_t
//
/*!
 * \since v.5.5.8
 * \brief An actual implementation of dispatcher with one working
 * thread and support of demand priorities.
 */
class dispatcher_t : public so_5::rt::dispatcher_t
	{
	public:
		dispatcher_t()
			:	m_work_thread{ m_demand_queue }
			{}

		//! \name Implementation of so_5::rt::dispatcher methods.
		//! \{
		virtual void
		start( so_5::rt::environment_t & /*env*/ ) override
			{
//FIXME: data source must be started here!
//				m_data_source.start( env );

				so_5::details::do_with_rollback_on_exception(
						[this] { m_work_thread.start(); },
						[this] {
//FIXME: data source must be stopped here!
// m_data_source.stop();
						} );
			}

		virtual void
		shutdown() override
			{
				m_demand_queue.stop();
			}

		virtual void
		wait() override
			{
				m_work_thread.join();

//FIXME: data source must be stopped here!
//				m_data_source.stop();
			}

		virtual void
		set_data_sources_name_base(
			const std::string & /*name_base*/ ) override
			{
//FIXME: base part of data source name must be set here!
//				m_data_source.set_data_sources_name_base( name_base, this );
			}
		//! \}

		/*!
		 * \since v.5.4.0
		 * \brief Get a binding information for an agent.
		 */
		so_5::rt::event_queue_t *
		get_agent_binding( priority_t priority )
			{
				return &m_demand_queue.event_queue_by_priority( priority );
			}

	private:
		//! Demand queue for the dispatcher.
		demand_queue_t m_demand_queue;

		//! Working thread for the dispatcher.
		work_thread_t m_work_thread;
	};

//
// binding_actions_mixin_t
//
/*!
 * \since v.5.5.8
 * \brief Implementation of binding actions to be reused
 * in various binder implementation.
 */
class binding_actions_mixin_t
	{
	protected :
		inline static so_5::rt::disp_binding_activator_t
		do_bind(
			dispatcher_t & disp,
			so_5::rt::agent_ref_t agent,
			priority_t priority )
			{
				auto result = [agent, &disp, priority]() {
					agent->so_bind_to_dispatcher(
							*(disp.get_agent_binding( priority )) );
				};

//FIXME: uncomment this when run-time data stats will be implemented.
#if 0
				// Dispatcher must know about yet another agent bound.
				disp.agent_bound();
#endif

				return result;
			}

		inline static void
		do_unbind(
			dispatcher_t & /*disp*/ )
			{
//FIXME: uncomment this when run-time data stats will be implemented.
#if 0
				// Dispatcher must know about yet another agent bound.
				disp.agent_unbound();
#endif
			}
	};

//
// disp_binder_t
//
/*!
 * \since v.5.5.8
 * \brief Dispatcher binder for dispatcher.
 */
class disp_binder_t
	:	public so_5::rt::disp_binder_t
	,	protected binding_actions_mixin_t
	{
	public:
		disp_binder_t(
			std::string disp_name,
			priority_t priority )
			:	m_disp_name( std::move( disp_name ) )
			,	m_priority( priority )
			{}

		virtual so_5::rt::disp_binding_activator_t
		bind_agent(
			so_5::rt::environment_t & env,
			so_5::rt::agent_ref_t agent ) override
			{
				using namespace so_5::disp::reuse;

				return do_with_dispatcher< dispatcher_t >(
					env,
					m_disp_name,
					[this, agent]( dispatcher_t & disp )
					{
						return do_bind( disp, std::move( agent ), m_priority );
					} );
			}

		virtual void
		unbind_agent(
			so_5::rt::environment_t & env,
			so_5::rt::agent_ref_t agent ) override
			{
				using namespace so_5::disp::reuse;

				do_with_dispatcher< dispatcher_t >( env, m_disp_name,
					[agent]( dispatcher_t & disp )
					{
						do_unbind( disp );
					} );
			}

	private:
		//! Name of the dispatcher to be bound to.
		const std::string m_disp_name;
		//! Priority to be used for binding.
		const priority_t m_priority;
	};

//
// private_dispatcher_binder_t
//

/*!
 * \since v.5.5.8
 * \brief A binder for the private %common_thread dispatcher.
 */
class private_dispatcher_binder_t
	:	public so_5::rt::disp_binder_t
	,	private binding_actions_mixin_t
	{
	public:
		explicit private_dispatcher_binder_t(
			//! A handle for private dispatcher.
			//! It is necessary to manage lifetime of the dispatcher instance.
			private_dispatcher_handle_t handle,
			//! A dispatcher instance to work with.
			dispatcher_t & instance,
			//! A priority to be used during binding.
			priority_t priority )
			:	m_handle( std::move( handle ) )
			,	m_instance( instance )
			,	m_priority( priority )
			{}

		virtual so_5::rt::disp_binding_activator_t
		bind_agent(
			so_5::rt::environment_t & /* env */,
			so_5::rt::agent_ref_t agent ) override
			{
				return do_bind( m_instance, std::move( agent ), m_priority );
			}

		virtual void
		unbind_agent(
			so_5::rt::environment_t & /*env*/,
			so_5::rt::agent_ref_t /*agent_ref*/ ) override
			{
				do_unbind( m_instance );
			}

	private:
		//! A handle for private dispatcher.
		/*!
		 * It is necessary to manage lifetime of the dispatcher instance.
		 */
		private_dispatcher_handle_t m_handle;
		//! A dispatcher instance to work with.
		dispatcher_t & m_instance;

		//! A priority to be used during binding.
		const priority_t m_priority;
	};

//
// real_private_dispatcher_t
//
/*!
 * \since v.5.5.8
 * \brief A real implementation of private_dispatcher interface.
 */
class real_private_dispatcher_t : public private_dispatcher_t
	{
	public :
		/*!
		 * Constructor creates a dispatcher instance and launces it.
		 */
		real_private_dispatcher_t(
			//! SObjectizer Environment to work in.
			so_5::rt::environment_t & env,
			//! Value for creating names of data sources for
			//! run-time monitoring.
			const std::string & data_sources_name_base )
			:	m_disp( new dispatcher_t() )
			{
				m_disp->set_data_sources_name_base( data_sources_name_base );
				m_disp->start( env );
			}

		/*!
		 * Destructors shuts an instance down and waits for it.
		 */
		~real_private_dispatcher_t()
			{
				m_disp->shutdown();
				m_disp->wait();
			}

		virtual so_5::rt::disp_binder_unique_ptr_t
		binder( priority_t priority ) override
			{
				return so_5::rt::disp_binder_unique_ptr_t(
						new private_dispatcher_binder_t(
								private_dispatcher_handle_t( this ),
								*m_disp,
								priority ) );
			}

	private :
		std::unique_ptr< dispatcher_t > m_disp;
	};

} /* namespace impl */

//
// private_dispatcher_t
//

private_dispatcher_t::~private_dispatcher_t()
	{}

//
// create_disp
//
SO_5_FUNC so_5::rt::dispatcher_unique_ptr_t
create_disp()
	{
		return so_5::rt::dispatcher_unique_ptr_t( new impl::dispatcher_t() );
	}

//
// create_private_disp
//
SO_5_FUNC private_dispatcher_handle_t
create_private_disp(
	so_5::rt::environment_t & env,
	const std::string & data_sources_name_base )
	{
		return private_dispatcher_handle_t(
				new impl::real_private_dispatcher_t(
						env, data_sources_name_base ) );
	}

//
// create_disp_binder
//
SO_5_FUNC so_5::rt::disp_binder_unique_ptr_t
create_disp_binder(
	const std::string & disp_name,
	priority_t priority )
	{
		return so_5::rt::disp_binder_unique_ptr_t( 
			new impl::disp_binder_t( disp_name, priority ) );
	}

} /* namespace common_thread */

} /* namespace prio */

} /* namespace disp */

} /* namespace so_5 */

