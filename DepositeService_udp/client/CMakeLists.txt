cmake_minimum_required(VERSION 2.8)
project(deposit_service_client_udp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c main.h packet_creator.c)
add_executable(deposit_service_client_udp ${SOURCE_FILES})