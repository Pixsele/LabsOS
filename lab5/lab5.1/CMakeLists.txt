cmake_minimum_required(VERSION 3.30)
project(lab5)

set(CMAKE_CXX_STANDARD 20)

add_executable(server Server.cpp)
target_link_libraries(server ws2_32)

add_executable(client Client.cpp)
target_link_libraries(client ws2_32)