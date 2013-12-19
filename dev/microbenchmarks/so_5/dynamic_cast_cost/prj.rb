require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	target "_microbench.dynamic_cast_cost"

	cpp_source "msg_consumer.cpp"
	cpp_source "main.cpp"
}

