require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj "ace/dll.rb"

	target "_microbench.demand_queue_1_bench_2"

	cpp_source "main.cpp"
}

