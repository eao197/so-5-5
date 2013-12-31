require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj "ace/dll.rb"

	target "_microbench.demand_queue_3_bench_1"

	cpp_source "main.cpp"
}

