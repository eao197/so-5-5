#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/msg_tracing'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/simple_msg_count/prj.ut.rb"
	required_prj "#{path}/simple_svc_count/prj.ut.rb"
	required_prj "#{path}/simple_svc_count_on_exception/prj.ut.rb"
	required_prj "#{path}/simple_msg_count_mpsc_no_limits/prj.ut.rb"
}
