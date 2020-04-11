#tcpmotor

cmake_minimum_required(VERSION 3.5)

project(TcpMotor)

add_definitions(-std=c++11)

aux_source_directory(. DIR_SRC_LISTS)

add_executable(
    tcpmotor
    ${DIR_SRC_LISTS}
    )

#ADD_LIBRARY(
#    tcpmotor
#    ${CORE_SRC_LISTS}
#    )
