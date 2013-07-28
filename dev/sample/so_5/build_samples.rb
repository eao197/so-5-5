require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "sample/so_5/build_samples.rb" ) {
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
	}
)
