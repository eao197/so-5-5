#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/message_limits'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/subscr_without_limit/prj.ut.rb"
	required_prj "#{path}/drop/prj.ut.rb"
}
