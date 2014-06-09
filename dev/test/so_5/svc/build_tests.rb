#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	required_prj( "test/so_5/svc/simple_svc/prj.ut.rb" )
	required_prj( "test/so_5/svc/svc_handler_exception/prj.ut.rb" )
	required_prj( "test/so_5/svc/no_svc_handlers/prj.ut.rb" )
	required_prj( "test/so_5/svc/several_svc_handlers/prj.ut.rb" )
	required_prj( "test/so_5/svc/svc_handler_not_called/prj.ut.rb" )
	required_prj( "test/so_5/svc/resending/prj.ut.rb" )
}
