require 'so_5/version'

MxxRu::Cpp::dll_target {

	rtl_mode( Mxx_ru::Cpp::RTL_SHARED )
	implib_path( "lib" )

	if 'mswin' == toolset.tag( 'target_os' )
		lib( 'ws2_32' )
	end

	required_prj( "ace/dll.rb" )

	target( "so." + So_5::VERSION )

	define( "SO_5_PRJ" )

	if "mswin" == toolset.tag( "target_os" )
		define( "SO_5__PLATFORM_REQUIRES_CDECL" )
	end

	# ./
	cpp_source( "exception.cpp" )

	# util.
	sources_root( "util" ) {
		cpp_source( "apply_throwing_strategy.cpp" )
	}

	# Run-time.
	sources_root( "rt" ) {

		cpp_source( "atomic_refcounted.cpp" )
		cpp_source( "state.cpp" )
		cpp_source( "nonempty_name.cpp" )

		cpp_source( "type_wrapper.cpp" )
		cpp_source( "message.cpp" )
		cpp_source( "message_ref.cpp" )

		cpp_source( "mbox.cpp" )
		cpp_source( "mbox_ref.cpp" )
		cpp_source( "subscription_key.cpp" )
		cpp_source( "subscription_bind.cpp" )

		cpp_source( "event_handler_caller.cpp" )
		cpp_source( "event_handler_caller_ref.cpp" )
		cpp_source( "event_caller_block.cpp" )
		cpp_source( "event_caller_block_ref.cpp" )
		cpp_source( "event_exception_logger.cpp" )
		cpp_source( "event_exception_handler.cpp" )

		cpp_source( "agent_ref.cpp" )
		cpp_source( "agent.cpp" )
		cpp_source( "agent_state_listener.cpp" )
		cpp_source( "agent_coop.cpp" )

		cpp_source( "so_environment.cpp" )

		cpp_source( "disp.cpp" )
		cpp_source( "disp_binder.cpp" )

		cpp_source( "so_layer.cpp" )

		cpp_source( "coop_listener.cpp" )

		sources_root( "impl" ) {
			cpp_source( "local_mbox.cpp" )
			cpp_source( "named_local_mbox.cpp" )
			cpp_source( "mbox_core.cpp" )
			cpp_source( "agent_core.cpp" )
			cpp_source( "disp_core.cpp" )
			cpp_source( "layer_core.cpp" )
			cpp_source( "local_event_queue.cpp" )
			cpp_source( "void_dispatcher.cpp" )
			cpp_source( "message_consumer_link.cpp" )
			cpp_source( "message_consumer_chain.cpp" )
			cpp_source( "message_distributor.cpp" )
			cpp_source( "state_listener_controller.cpp" )

			cpp_source( "so_environment_impl.cpp" )

			sources_root( "coop_dereg" ){
				cpp_source( "coop_dereg_executor_thread.cpp" )
				cpp_source( "dereg_demand_queue.cpp" )
			}
		}
	}

	sources_root( "disp" ) {
		sources_root( "one_thread" ) {
			sources_root( "impl" ) {
				cpp_source( "disp.cpp" )
				cpp_source( "disp_binder.cpp" )
			}
			cpp_source( "pub.cpp" )
		}

		sources_root( "active_obj" ) {
			sources_root( "impl" ) {
				cpp_source( "disp.cpp" )
				cpp_source( "disp_binder.cpp" )
			}
			cpp_source( "pub.cpp" )
		}

		sources_root( "active_group" ) {
			sources_root( "impl" ) {
				cpp_source( "disp.cpp" )
				cpp_source( "disp_binder.cpp" )
			}
			cpp_source( "pub.cpp" )
		}

		sources_root( "reuse" ) {
			sources_root( "work_thread" ) {
				cpp_source( "work_thread.cpp" )
			}
		}
	}

	# Timer thread.
	sources_root( "timer_thread" ) {
		cpp_source( "timer_thread.cpp" )
		cpp_source( "timer_act.cpp" )
		cpp_source( "timer_id.cpp" )

		sources_root( "ace_timer_queue_adapter"){
			cpp_source( "pub.cpp" )

			sources_root( "impl" ) {
				cpp_source( "event_handler.cpp" )
				cpp_source( "timer_thread.cpp" )
			}
		}
	}

}
