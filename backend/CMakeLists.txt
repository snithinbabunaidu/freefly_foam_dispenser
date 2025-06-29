cmake_minimum_required(VERSION 3.10.2)
project(drone_control)
set(CMAKE_CXX_STANDARD 17)

set(MAVSDK_ROOT "C:/mavsdk")
find_package(MAVSDK REQUIRED HINTS ${MAVSDK_ROOT})

add_executable(test_connection test_conn.cpp)
target_link_libraries(test_connection MAVSDK::mavsdk)