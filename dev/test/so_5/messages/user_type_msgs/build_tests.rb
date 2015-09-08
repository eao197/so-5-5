#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/messages/user_type_msgs'

	required_prj( "#{path}/simple_msgs/prj.ut.rb" )
	required_prj( "#{path}/simple_svc/prj.ut.rb" )
}
