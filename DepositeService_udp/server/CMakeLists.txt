cmake_minimum_required(VERSION 2.8)
project(deposit_service_server_udp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c main.h
        list_of_clients.c
        console_thread.c
        list_of_deposits.c client_thread.c packet_creator.c receiveing_thread.c )
add_executable(deposit_service_server_udp ${SOURCE_FILES})