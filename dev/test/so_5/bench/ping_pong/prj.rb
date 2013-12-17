require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj "ace/dll.rb"
	required_prj "so_5/prj.rb"
	required_prj "cls_3/prj.rb"

	target "_test.bench.ping_pong"

	cpp_source "main.cpp"
}

