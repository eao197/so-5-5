#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/message_limits'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/subscr_without_limit/prj.ut.rb"
	required_prj "#{path}/drop/prj.ut.rb"
	required_prj "#{path}/abort_app/mc_mbox/prj.ut.rb"
	required_prj "#{path}/abort_app/sc_mbox/prj.ut.rb"
	required_prj "#{path}/redirect_msg/mc_mbox/prj.ut.rb"
	required_prj "#{path}/redirect_msg/sc_mbox/prj.ut.rb"
	required_prj "#{path}/redirect_svc/mc_mbox/prj.ut.rb"
	required_prj "#{path}/redirect_svc/sc_mbox/prj.ut.rb"
}
