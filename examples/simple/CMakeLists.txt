add_executable(simple_chargen chargen/main.cc)
target_link_libraries(simple_chargen muduo_net)

add_executable(simple_daytime daytime/main.cc)
target_link_libraries(simple_daytime muduo_net)

add_executable(simple_discard discard/main.cc)
target_link_libraries(simple_discard muduo_net)

add_executable(simple_echo ./echo.cc)
target_link_libraries(simple_echo muduo_net)

add_executable(simple_time time/main.cc)
target_link_libraries(simple_time muduo_net)

add_executable(simple_allinone
  allinone/allinone.cc
  )
target_link_libraries(simple_allinone muduo_net)

add_executable(simple_timeclient timeclient/timeclient.cc)
target_link_libraries(simple_timeclient muduo_net)

add_executable(simple_chargenclient chargenclient/chargenclient.cc)
target_link_libraries(simple_chargenclient muduo_net)

