#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/msg_tracing'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/simple_msg_count/prj.ut.rb"
}
