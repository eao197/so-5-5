require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
	required_prj( "sample/so_5/hello_world/prj.rb" )
	required_prj( "sample/so_5/hello_evt_handler/prj.rb" )
	required_prj( "sample/so_5/hello_all/prj.rb" )
	required_prj( "sample/so_5/hello_delay/prj.rb" )
	required_prj( "sample/so_5/hello_periodic/prj.rb" )
	required_prj( "sample/so_5/chstate/prj.rb" )
	required_prj( "sample/so_5/disp/prj.rb" )
	required_prj( "sample/so_5/coop_listener/prj.rb" )
	required_prj( "sample/so_5/exception_logger/prj.rb" )
	required_prj( "sample/so_5/exception_handler/prj.rb" )
	required_prj( "sample/so_5/coop_notification/prj.rb" )
	required_prj( "sample/so_5/coop_user_resources/prj.rb" )
	required_prj( "sample/so_5/layer/prj.rb" )
	required_prj( "sample/so_5/mboxes/prj.rb" )
	required_prj( "sample/so_5/parent_coop/prj.rb" )
	required_prj( "sample/so_5/chameneos_simple/prj.rb" )
	required_prj( "sample/so_5/svc/hello/prj.rb" )
}
