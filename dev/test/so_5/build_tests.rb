#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "test/so_5/build_tests.rb" ) {

		required_prj( "test/so_5/bench/work_thread_speed/prj.rb" )

		required_prj( "test/so_5/timer_thread/single_delayed/prj.ut.rb" )
		required_prj( "test/so_5/timer_thread/single_periodic/prj.ut.rb" )
		required_prj( "test/so_5/timer_thread/single_timer_zero_delay/prj.ut.rb" )

		required_prj( "test/so_5/disp/binder/bind_to_disp_1/prj.ut.rb" )
		required_prj( "test/so_5/disp/binder/bind_to_disp_2/prj.ut.rb" )
		required_prj( "test/so_5/disp/binder/bind_to_disp_3/prj.ut.rb" )
		required_prj( "test/so_5/disp/binder/bind_to_disp_error_no_disp/prj.ut.rb" )
		required_prj( "test/so_5/disp/binder/bind_to_disp_error_disp_type_mismatch/prj.ut.rb" )

		required_prj( "test/so_5/event_handler/subscribe_errors/prj.ut.rb" )

		required_prj( "test/so_5/messages/three_messages/prj.ut.rb" )
		required_prj( "test/so_5/messages/resend_message/prj.ut.rb" )
		required_prj( "test/so_5/messages/store_and_resend_later/prj.ut.rb" )

		required_prj( "test/so_5/state/change_state/prj.ut.rb" )

		required_prj( "test/so_5/coop/duplicate_name/prj.ut.rb" )
		required_prj( "test/so_5/coop/reg_some_and_stop_1/prj.ut.rb" )
		required_prj( "test/so_5/coop/reg_some_and_stop_2/prj.ut.rb" )
		required_prj( "test/so_5/coop/reg_some_and_stop_3/prj.ut.rb" )
		required_prj( "test/so_5/coop/throw_on_define_agent/prj.ut.rb" )
		required_prj( "test/so_5/coop/throw_on_bind_to_disp/prj.ut.rb" )
		required_prj( "test/so_5/coop/parent_child_1/prj.ut.rb" )

		required_prj( "test/so_5/mbox/subscribe_when_deregistered/prj.ut.rb" )


		required_prj( "test/so_5/layer/layer_init/prj.ut.rb" )
		required_prj( "test/so_5/layer/layer_query/prj.ut.rb" )
		required_prj( "test/so_5/layer/extra_layer_query/prj.ut.rb" )
		required_prj( "test/so_5/layer/extra_layer_errors/prj.ut.rb" )

		required_prj( "test/so_5/api/run_so_environment/prj.ut.rb" )
	}
)
