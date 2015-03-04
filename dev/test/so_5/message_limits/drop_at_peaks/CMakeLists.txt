add_executable(_unit.test.message_limits.drop_at_peaks main.cpp)
target_link_libraries(_unit.test.message_limits.drop_at_peaks so.${SO_5_VERSION})
add_test(_unit.test.message_limits.drop_at_peaks _unit.test.message_limits.drop_at_peaks)
