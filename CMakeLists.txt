cmake_minimum_required(VERSION 3.20)
project(broadcast-net C)
find_package(OpenSSL)
include_directories(include/)

add_library(bcnet SHARED
    src/auth.c
)

if(WIN32)
    target_sources(bcnet
        src/win32/bcnet.c
    )
else()
    target_sources(bcnet
    src/unix/bcnet.c
    )
endif()

add_executable("test.c")