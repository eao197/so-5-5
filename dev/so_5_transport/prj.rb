require 'mxx_ru/cpp'

require 'so_5_transport/version.rb'

MxxRu::Cpp::dll_target {

	required_prj "ace/dll.rb"

	required_prj 'oess_2/defs/prj.rb'
	required_prj 'oess_2/io/prj.rb'

	required_prj 'so_5/prj.rb'

	target( "so_5_transport." + So_5_Transport::VERSION )
	implib_path 'lib'

	define( "SO_5_TRANSPORT_PRJ" )

	# ./ace
	sources_root( "ace" ){
		cpp_source( "reactors.cpp" )
	}

	# ./
	cpp_source( "reactor_layer.cpp" )


	cpp_source( "raw_block.cpp" )
	cpp_source( "scattered_block_stream.cpp" )
	cpp_source( "channel_stream.cpp" )
	cpp_source( "io_trx.cpp" )
	cpp_source( "channel_io.cpp" )

	cpp_source( "channel_params.cpp" )

	cpp_source( "channel_controller.cpp" )
	cpp_source( "ifaces.cpp" )

	cpp_source( "messages.cpp" )

	cpp_source( "a_transport_agent.cpp" )
	cpp_source( "a_channel_base.cpp" )
	cpp_source( "a_client_base.cpp" )
	cpp_source( "a_server_base.cpp" )

	# # ./impl
	# sources_root( "impl" ){
		# cpp_source( "helpers.cpp" )
	# }

	# ./socket
	sources_root( "socket" ){
		sources_root( "impl" ){
			cpp_source( "acceptor_controller.cpp" )
			cpp_source( "connector_controller.cpp" )
			cpp_source( "io_timestamps.cpp" )
			cpp_source( "channel_controller_impl.cpp" )
			cpp_source( "svc_handler.cpp" )
		}
		cpp_source( "pub.cpp" )
	}

}
