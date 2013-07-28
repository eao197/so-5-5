require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
	required_prj 'build.rb'

	required_prj 'test/so_5/build_tests.rb'
	required_prj 'test/so_5_transport/build_tests.rb'
	required_prj 'test/mbapi_4/build_tests.rb'

	required_prj 'sample/so_5/build_samples.rb'
	required_prj 'sample/so_5_transport/build_samples.rb'
	required_prj 'sample/mbapi_4/build_samples.rb'
}
