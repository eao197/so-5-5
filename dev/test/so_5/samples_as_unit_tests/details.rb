require 'mxx_ru/binary_unittest'

def setup_sample_as_unit_test(sample_name = nil)
  ut_name = File.basename( /^(.+):\d/.match( caller(1,1)[0] )[1] )
  sample_name = File.basename( ut_name, '.ut.rb' ) unless sample_name

  ut_path     = 'test/so_5/samples_as_unit_tests'
  sample_path = "sample/so_5/#{sample_name}"

  MxxRu::setup_target(
    MxxRu::BinaryUnittestTarget.new(
      "#{ut_path}/#{ut_name}",
      "#{sample_path}/prj.rb" )
  )
end
