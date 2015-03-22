#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

path = 'test/so_5/internal_stats'

MxxRu::Cpp::composite_target {

	required_prj "#{path}/simple_turn_on/prj.ut.rb"
}
