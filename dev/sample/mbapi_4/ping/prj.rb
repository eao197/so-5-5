require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "sample/mbapi_4/ping/prj.rb" ) {
		required_prj( "sample/mbapi_4/ping/ping_srv.rb" )
		required_prj( "sample/mbapi_4/ping/ping_cln.rb" )
		required_prj( "sample/mbapi_4/ping/ping_proxy.rb" )
	}
)
