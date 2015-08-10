set(UNITTEST _unit.test.disp.prio_common_thread.simple_seq1)
add_executable(${UNITTEST} main.cpp)
target_link_libraries(${UNITTEST} so.${SO_5_VERSION})
add_test(${UNITTEST} ${UNITTEST})
