cmake_minimum_required(VERSION 3.14)
project(zabbix_sender)

set(CMAKE_CXX_STANDARD 11)

add_definitions(
        -O2 -g -W -Wall
        -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
        -Wno-deprecated -Woverloaded-virtual -Wwrite-strings
)

aux_source_directory(src DIR_SRCS)

add_library(zabbix_sender ${DIR_SRCS})