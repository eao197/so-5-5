set(UNITTEST _unit.test.disp.prio_common_thread.dereg_when_queue_not_empty)
add_executable(${UNITTEST} main.cpp)
target_link_libraries(${UNITTEST} so.${SO_5_VERSION})
add_test(${UNITTEST} ${UNITTEST})
