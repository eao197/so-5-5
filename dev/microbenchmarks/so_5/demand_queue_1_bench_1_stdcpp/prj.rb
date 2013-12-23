require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	target "_microbench.demand_queue_1_bench_1_stdcpp"

	cpp_source "main.cpp"

	if 'gcc' == toolset.name
		compiler_option '-pthread'
		linker_option '-pthread'
	end
}

