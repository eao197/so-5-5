add_executable(_unit.test.message_limits.drop main.cpp)
target_link_libraries(_unit.test.message_limits.drop so.${SO_5_VERSION})
add_test(_unit.test.message_limits.drop _unit.test.message_limits.drop)
