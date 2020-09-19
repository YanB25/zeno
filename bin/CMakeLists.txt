include_directories(${Boost_INCLUDE_DIRS} ${AIO_INCLUDE_DIR})
link_libraries(${Boost_LIBRARIES} pthread ${AIO_LIBRARIES} helloworld_obj)

add_executable(server server.cpp)

add_executable(multithread-server multithread-server.cpp)

add_executable(client client.cpp)