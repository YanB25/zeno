add_executable(main main.cpp)
target_link_libraries(main PRIVATE helloworld_obj)

add_executable(log_test log_test.cpp)
target_link_libraries(log_test PRIVATE helloworld_obj)

add_executable(smart_test smart_test.cpp)
target_link_libraries(smart_test PRIVATE helloworld_obj)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE helloworld_obj ${Boost_LIBRARIES} pthread)
target_include_directories(server PUBLIC ${Boost_INCLUDE_DIRS})

add_executable(multithread-server multithread-server.cpp)
target_link_libraries(multithread-server PRIVATE helloworld_obj ${Boost_LIBRARIES} pthread)
target_include_directories(multithread-server PUBLIC ${Boost_INCLUDE_DIRS})

add_executable(client client.cpp)
target_link_libraries(client PRIVATE helloworld_obj ${Boost_LIBRARIES} pthread)
target_include_directories(client PUBLIC ${Boost_INCLUDE_DIRS})