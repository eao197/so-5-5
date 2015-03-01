add_executable(_unit.test.message_limits.subscr_without_limit main.cpp)
target_link_libraries(_unit.test.message_limits.subscr_without_limit so.${SO_5_VERSION})
add_test(_unit.test.message_limits.subscr_without_limit _unit.test.message_limits.subscr_without_limit)
