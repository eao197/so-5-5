require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {
	required_prj( "ace/dll.rb" )
	required_prj( "so_5/prj.rb" )

	required_prj( "ace/dll.rb" )
	required_prj( "so_5/prj.rb" )

	required_prj( "oess_2/defs/prj.rb" )
	required_prj( "oess_2/stdsn/prj.rb" )

	required_prj( "mbapi_4/defs/prj.rb" )
	required_prj( "mbapi_4/prj.rb" )


	target( "sample.mbapi_4.binary_message" )

	ddl = generator( Oess_2::Util_cpp_serializer2::Gen.new( self ) )
	ddl.repository_name( "sample__mbapi_4__binary_messages__messages" )
	ddl.cpp_file( "messages.ddl.cpp" )

	ddl.ddl_file( "messages.ddl" )


	cpp_source( "main.cpp" )
}
