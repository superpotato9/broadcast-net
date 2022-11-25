cmake_minimum_required(VERSION 3.20)
project(broadcast-net C)
find_package(OpenSSL)
include_directories(include/)

add_library(bcnet SHARED
    src/bcnet.c
)