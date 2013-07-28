require 'mxx_ru/cpp'

require 'mbapi_4/version.rb'

MxxRu::Cpp::dll_target {

	required_prj "ace/dll.rb"
	required_prj "so_5/prj.rb"

	target( "mbapi.defs" + Mbapi_4::VERSION )
	implib_path 'lib'


	define( "MBAPI_4_DEFS_PRJ" )

	cpp_source( "stagepoint.cpp" )
	cpp_source( "endpoint.cpp" )
	cpp_source( "handshaking_params.cpp" )

	cpp_source( "channel_uid_wrapper.cpp" )
	cpp_source( "mbapi_node_uid.cpp" )
	cpp_source( "oess_id_wrapper.cpp" )

	sources_root( "impl" ){
		cpp_source( "check_point_name.cpp" )
	}

}
