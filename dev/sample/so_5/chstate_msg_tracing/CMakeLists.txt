set(SAMPLE sample.so_5.chstate_msg_tracing)
add_executable(${SAMPLE} main.cpp key_value_storage.cpp)
target_link_libraries(${SAMPLE} so.${SO_5_VERSION})
