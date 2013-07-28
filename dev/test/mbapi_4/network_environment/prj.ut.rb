require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/mbapi_4/network_environment/prj.ut.rb",
		"test/mbapi_4/network_environment/prj.rb" )
)
