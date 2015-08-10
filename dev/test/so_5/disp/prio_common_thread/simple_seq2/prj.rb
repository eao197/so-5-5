require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'so_5/prj.rb'

	target '_unit.test.disp.prio_common_thread.simple_seq2'

	cpp_source 'main.cpp'
}

