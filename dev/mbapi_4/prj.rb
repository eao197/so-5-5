require 'mxx_ru/cpp'

require 'mbapi_4/version.rb'

require 'oess_2/util_cpp_serializer2/gen'

MxxRu::Cpp::dll_target {

	required_prj "ace/dll.rb"

	required_prj 'oess_2/defs/prj.rb'
	required_prj 'oess_2/io/prj.rb'
	required_prj 'oess_2/stdsn/prj.rb'

	required_prj 'so_5/prj.rb'
	required_prj 'so_5_transport/prj.rb'

	target( "mbapi" + Mbapi_4::VERSION )
	implib_path 'lib'

	define( "MBAPI_4_PRJ" )

	required_prj 'mbapi_4/defs/prj.rb'
	required_prj 'mbapi_4/impl/infrastructure/prj.rb'
	required_prj 'mbapi_4/zlib/prj.rb'

	sources_root( "proto" ){
		cpp_source( "package_header.cpp" )

		ddl = generator( Oess_2::Util_cpp_serializer2::Gen.new( self ) )
		ddl.repository_name( "mbapi_4_proto" )
		ddl.cpp_file( "generated.ddl.cpp" )

		cpp_source( "req_info.cpp" )
		ddl.ddl_file( "req_info.ddl" )

		cpp_source( "ping_info.cpp" )
		ddl.ddl_file( "ping_info.ddl" )

		cpp_source( "sync_tables_info.cpp" )
		ddl.ddl_file( "sync_tables_info.ddl" )

		cpp_source( "send_msg_info.cpp" )
		ddl.ddl_file( "send_msg_info.ddl" )

		cpp_source( "handshake_info.cpp" )
		ddl.ddl_file( "handshake_info.ddl" )

		cpp_source( "common_ddl.cpp" )
	}

	sources_root( "comm" ){
		cpp_source( "transmit_info.cpp" )
		cpp_source( "a_mbapi_incoming_channel.cpp" )
		cpp_source( "a_mbapi_outgoing_channel.cpp" )

		sources_root( "impl" ){
			cpp_source( "get_layer_impl.cpp" )
			cpp_source( "traffic_transformator.cpp" )
			cpp_source( "iochannel_data_processor.cpp" )
		}
	}

	sources_root( "impl" ){
		cpp_source( "iosession.cpp" )
		cpp_source( "mbapi_layer_impl.cpp" )
		cpp_source( "stagepoint_bind_impl.cpp" )
	}

	cpp_source( "message.cpp" )
	cpp_source( "mbapi_layer.cpp" )
	cpp_source( "stagepoint_bind.cpp" )
	cpp_source( "endpoint_bind.cpp" )
}
