require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj( "sample/mbapi_4/ping/messages.rb" )
	
	required_prj( "ace/dll.rb" )
	required_prj( "so_5/prj.rb" )
	required_prj( "so_5_transport/prj.rb" )

	required_prj( "mbapi_4/defs/prj.rb" )
	required_prj( "mbapi_4/prj.rb" )

	target( "sample.mbapi_4.ping.cln" )

	cpp_source( "ping_cln.cpp" )
}
