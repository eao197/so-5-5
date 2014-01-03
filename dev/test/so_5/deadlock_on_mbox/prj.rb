require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj "ace/dll.rb"

	target "_test.deadlock_on_mbox"

	cpp_source "main.cpp"
}

