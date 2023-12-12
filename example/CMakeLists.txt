cmake_minimum_required(VERSION 3.25)
project(concatenator)
find_package(sdbus-c++ REQUIRED)
set(CMAKE_BUILD_TYPE Debug)

add_executable(server_side server.cpp)
target_link_libraries(server_side sdbus-c++)

add_executable(client_side client.cpp)
target_link_libraries(client_side sdbus-c++)
target_compile_options(client_side PRIVATE -g)