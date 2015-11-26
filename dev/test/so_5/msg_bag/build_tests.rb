#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/msg_bag'

	required_prj( "#{path}/simple/prj.ut.rb" )
	required_prj( "#{path}/simple_svc/prj.ut.rb" )
	required_prj( "#{path}/close_bag/prj.ut.rb" )
}
