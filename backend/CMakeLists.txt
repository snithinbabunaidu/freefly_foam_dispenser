cmake_minimum_required(VERSION 3.10.2)
project(drone_control)
set(CMAKE_CXX_STANDARD 17)

set(MAVSDK_ROOT "C:/mavsdk")
find_package(MAVSDK REQUIRED HINTS ${MAVSDK_ROOT})

add_executable(backend_flight_module test_conn.cpp)
target_link_libraries(backend_flight_module MAVSDK::mavsdk)