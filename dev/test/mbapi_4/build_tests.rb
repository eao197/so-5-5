#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "test/mbapi_4/build_tests.rb" ) {
		required_prj( "test/mbapi_4/bench/prj.rb" )
		required_prj( "test/mbapi_4/bench2/prj.rb" )
		required_prj( "test/mbapi_4/bench3/prj.rb" )
		required_prj( "test/mbapi_4/network_environment/prj.ut.rb" )
	}
)
