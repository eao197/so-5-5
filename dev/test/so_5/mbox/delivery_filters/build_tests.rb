#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/mbox/delivery_filters'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/simple/prj.ut.rb"
	required_prj "#{path}/filter_no_subscriptions/prj.ut.rb"
	required_prj "#{path}/filter_on_mpsc_mbox/prj.ut.rb"
	required_prj "#{path}/dereg_subscriber/prj.ut.rb"
}
