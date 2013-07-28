require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj "ace/dll.rb"
	required_prj "so_5/prj.rb"
	required_prj "cls_3/prj.rb"

	target "_test.bench.work_thread_speed"

	cpp_source "a_bench_receiver.cpp"
	cpp_source "a_bench_sender.cpp"
	cpp_source "a_bench_arbiter.cpp"
	cpp_source "tag_bench_cfg.cpp"
	cpp_source "main.cpp"
}

