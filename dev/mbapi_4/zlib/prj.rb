require 'mxx_ru/cpp'

require 'mbapi_4/version'

Mxx_ru::setup_target(
	Mxx_ru::Cpp::Lib_target.new( "mbapi_4/zlib/prj.rb" ) {
		target_root( "lib" )

		target( "mbapi_4.zlib" + Mbapi_4::VERSION )

		c_source( "adler32.c" )
		c_source( "compress.c" )
		c_source( "crc32.c" )
		c_source( "deflate.c" )
		c_source( "gzclose.c" )
		c_source( "gzguts.h" )
		c_source( "gzlib.c" )
		c_source( "gzread.c" )
		c_source( "gzwrite.c" )
		c_source( "infback.c" )
		c_source( "inffast.c" )
		c_source( "inflate.c" )
		c_source( "inftrees.c" )
		c_source( "trees.c" )
		c_source( "uncompr.c" )
		c_source( "zutil.c" )

		include_path( "mbapi_4/zlib", Mxx_ru::Cpp::Target::OPT_UPSPREAD )
	}
)
