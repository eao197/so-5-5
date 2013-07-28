require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj( "ace/dll.rb" )
	required_prj( "so_5/prj.rb" )
	required_prj( "so_5_transport/prj.rb" )

	target( "sample.so_5_transport.raw_channel.tcp_cln" )

	cpp_source( "tcp_cln.cpp" )
}
