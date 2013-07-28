require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj( "test/mbapi_4/bench3/messages.rb" )
	
	required_prj( "ace/dll.rb" )
	required_prj( "so_5/prj.rb" )
	required_prj( "so_5_transport/prj.rb" )

	required_prj( "mbapi_4/defs/prj.rb" )
	required_prj( "mbapi_4/prj.rb" )

	target( "_test.mbapi_4.bench3" )

	cpp_source( "main.cpp" )
}
