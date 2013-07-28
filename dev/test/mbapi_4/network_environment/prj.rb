require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj( "ace/dll.rb" )
	required_prj( "mbapi_4/prj.rb" )

	target( "_unit.test.mbapi_4.network_environment" )

	required_prj 'mbapi_4/defs/prj.rb'
	required_prj 'mbapi_4/impl/infrastructure/prj.rb'

	cpp_source( "checkers.cpp" )
	cpp_source( "shift_to_next_stage.cpp" )
	cpp_source( "local_node_updates.cpp" )
	cpp_source( "multiple_channel_updates.cpp" )
	cpp_source( "single_channel_updates.cpp" )
	cpp_source( "main.cpp" )
}

