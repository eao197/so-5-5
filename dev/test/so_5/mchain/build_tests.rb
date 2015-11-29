#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/mchain'

	required_prj( "#{path}/infinite_wait/prj.ut.rb" )
	required_prj( "#{path}/simple/prj.ut.rb" )
	required_prj( "#{path}/simple_svc/prj.ut.rb" )
	required_prj( "#{path}/close_chain/prj.ut.rb" )
	required_prj( "#{path}/adv_receive/prj.ut.rb" )
}
