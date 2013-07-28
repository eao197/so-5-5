require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "sample/mbapi_4/build_samples.rb" ) {
		required_prj( "sample/mbapi_4/ping/prj.rb" )
		required_prj( "sample/mbapi_4/stages/prj.rb" )
		required_prj( "sample/mbapi_4/binary_messages/prj.rb" )
	}
)
