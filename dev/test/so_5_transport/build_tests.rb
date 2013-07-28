#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "test/so_5_transport/build_tests.rb" ) {
		required_prj( "test/so_5_transport/scattered_block_stream/prj.ut.rb" )
		required_prj( "test/so_5_transport/channel_stream/prj.ut.rb" )
	}
)
