require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj( "ace/dll.rb" )
	required_prj( "oess_2/defs/prj.rb" )
	required_prj( "oess_2/io/prj.rb" )
	required_prj( "so_5/prj.rb" )
	required_prj( "so_5_transport/prj.rb" )

	target( "_unit.test.so_5_transport.channel_stream" )

	cpp_source( "main.cpp" )
}

