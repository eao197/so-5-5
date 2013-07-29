require 'rubygems'

gem 'Mxx_ru', '>= 1.3.0'

require 'mxx_ru/cpp'
require 'oess_2/util_cpp_serializer2/gen.rb'

MxxRu::Cpp::lib_target {

	target 'lib/sample.mbapi_4.stages.messages'

	required_prj 'oess_2/defs/prj.rb'
	required_prj 'oess_2/stdsn/prj.rb'

	ddl = generator( Oess_2::Util_cpp_serializer2::Gen.new( self ) )
	ddl.repository_name( "sample__mbapi_4__stages__messages" )
	ddl.cpp_file( "generated.ddl.cpp" )

	ddl.ddl_file( "messages.ddl" )

	cpp_source( "messages.cpp" )
}

