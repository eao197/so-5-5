require 'mxx_ru/cpp'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Composite_target.new( Mxx_ru::BUILD_ROOT ) {

		toolset.force_cpp0x_std
		global_include_path "."

		if toolset.name == 'gcc' && toolset.tag('gcc_port', 'unix') == 'cygwin'
			global_define "ACE_HAS_CUSTOM_EXPORT_MACROS=0"
		end

		default_runtime_mode( MxxRu::Cpp::RUNTIME_RELEASE )
		MxxRu::enable_show_brief

		required_prj( "ace/ace_lib_unpacker.rb" )
		required_prj( "so_5/prj.rb" )
		required_prj( "so_5_transport/prj.rb" )
		required_prj( "mbapi_4/prj.rb" )
	}
)
