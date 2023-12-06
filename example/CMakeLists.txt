cmake_minimum_required(VERSION 3.25)
project(concatenator)
find_package(sdbus-c++ REQUIRED)

add_executable(server_side server.cpp)
target_link_libraries(server_side sdbus-c++)

add_executable(client_side client.cpp)
target_link_libraries(client_side sdbus-c++)