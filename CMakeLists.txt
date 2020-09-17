cmake_minimum_required(VERSION 3.12.4)

project (zeno)

option(BUILD_BINARIES "Build the binaries in {project}/bin directory" ON)
option(BUILD_STATIC_LIBS "Build the static library" ON)
option(BUILD_SHARED_LIBS "Build the shared library" ON)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# dir to include
set(HELLOWORLD_INCLUDE_DIRECTORIES include)

add_compile_options("-Wall" "-W" "-Wextra" "-fPIC")

# for boost::asio
find_package(Boost
    1.71
    REQUIRED
    COMPONENTS
    system)

# headers to install
file(GLOB_RECURSE HELLOWORLD_PUBLIC_HEADER include/*)

# source for the lib
file(GLOB_RECURSE HELLOWORLD_SOURCE src/*)
add_library(helloworld_obj OBJECT ${HELLOWORLD_SOURCE})
target_include_directories(helloworld_obj PUBLIC ${HELLOWORLD_INCLUDE_DIRECTORIES})

if (${BUILD_BINARIES})
    add_subdirectory(bin)
endif()

if (${BUILD_STATIC_LIBS})
    add_library(zeno-static STATIC $<TARGET_OBJECTS:helloworld_obj>)

    # install instructions
    set_target_properties(zeno-static PROPERTIES PUBLIC_HEADER "${HELLOWORLD_PUBLIC_HEADER}")
    install(TARGETS zeno-static PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zeno)
endif()

if (${BUILD_SHARED_LIBS})
    add_library(zeno-dynamic SHARED $<TARGET_OBJECTS:helloworld_obj>)

    # install instructions
    set_target_properties(zeno-dynamic PROPERTIES PUBLIC_HEADER "${HELLOWORLD_PUBLIC_HEADER}")
    install(TARGETS zeno-dynamic PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zeno)
endif()
