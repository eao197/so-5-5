#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/msg_bag'

	required_prj( "#{path}/simple/prj.ut.rb" )
}
