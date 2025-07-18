cmake_minimum_required(VERSION 3.10.2)
project(drone_control)
set(CMAKE_CXX_STANDARD 17)

set(MAVSDK_ROOT "C:/mavsdk")
find_package(MAVSDK REQUIRED HINTS ${MAVSDK_ROOT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(backend_flight_module swarm_backend.cpp)
target_link_libraries(backend_flight_module MAVSDK::mavsdk ws2_32 wsock32)