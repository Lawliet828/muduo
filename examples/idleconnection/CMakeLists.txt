if(Boost_FOUND)
  add_executable(idleconnection_echo idleconnection_echo.cc)
  target_link_libraries(idleconnection_echo muduo_net)
endif()

add_executable(idleconnection_echo2 sortedlist.cc)
target_link_libraries(idleconnection_echo2 muduo_net)
