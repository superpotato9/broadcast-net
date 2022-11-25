cmake_minimum_required(VERSION 3.20)
project(brodcast-net C)

include_directories(include/)

add_library(bcnet SHARED
    src/bcnet.c
)