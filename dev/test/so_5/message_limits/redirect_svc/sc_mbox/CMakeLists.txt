add_executable(_unit.test.message_limits.redirect_svc.sc_mbox main.cpp)
target_link_libraries(_unit.test.message_limits.redirect_svc.sc_mbox so.${SO_5_VERSION})
add_test(_unit.test.message_limits.redirect_svc.sc_mbox _unit.test.message_limits.redirect_svc.sc_mbox)
