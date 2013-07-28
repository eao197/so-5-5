require 'rubygems'

gem 'Mxx_ru', '>= 1.3.0'

require 'mxx_ru/cpp'

MxxRu::Cpp::lib_target {
	required_prj "ace/dll.rb"

	# Define your target name here.
	target 'lib/mbapi_4.impl.infrastructure'

	required_prj 'mbapi_4/defs/prj.rb'
	required_prj 'so_5/prj.rb'

	cpp_source( "availability_tables.cpp" )
	cpp_source( "channel_info.cpp" )
	cpp_source( "network_environment.cpp" )
}

