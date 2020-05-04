#client

cmake_minimum_required(VERSION 3.5)

project(TcpMotor)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

#add_definitions(-std=c++11)

#aux_source_directory(. DIR_SRC_LISTS)

add_executable(
    client

    ../test_client.cpp
    #${DIR_SRC_LISTS}
    )

#ADD_LIBRARY(
#    client
#    ${CORE_SRC_LISTS}
#    )
