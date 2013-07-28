require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( "sample/so_5_transport/raw_channel/prj.rb" ) {
		required_prj( "sample/so_5_transport/raw_channel/tcp_srv.rb" )
		required_prj( "sample/so_5_transport/raw_channel/tcp_cln.rb" )
	}
)
