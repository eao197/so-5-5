#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/state'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/change_state/prj.ut.rb"
	required_prj "#{path}/composite_state_change/prj.ut.rb"
	required_prj "#{path}/enter_exit_handlers/prj.ut.rb"
	required_prj "#{path}/nesting_deep/prj.ut.rb"
	required_prj "#{path}/parent_state_handler/prj.ut.rb"
	required_prj "#{path}/state_history/prj.ut.rb"
}
