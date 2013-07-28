require 'rubygems'

gem 'Mxx_ru', '>= 1.3.0'

require 'mxx_ru/cpp'

MxxRu::Cpp::lib_target {

	target 'lib/test.mbapi_4.bench3.messages'

	required_prj 'oess_2/defs/prj.rb'
	required_prj 'oess_2/stdsn/prj.rb'

	ddl = generator( Oess_2::Util_cpp_serializer2::Gen.new( self ) )
	ddl.repository_name( "test__mbapi_4__bench3__messages" )
	ddl.cpp_file( "generated.ddl.cpp" )

	ddl.ddl_file( "messages.ddl" )

	cpp_source( "messages.cpp" )
}
